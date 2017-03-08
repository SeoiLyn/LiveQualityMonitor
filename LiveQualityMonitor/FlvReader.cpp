#include "FlvReader.h"

std::string FlvReader::video_dst_filename = "/home/water/temp/dump.yuv";

FlvReader::FlvReader()
: fmt_ctx(NULL)
, video_dec_ctx(NULL)
, audio_dec_ctx(NULL)
, video_stream(NULL)
, audio_stream(NULL)
, video_stream_idx(-1)
, audio_stream_idx(-1)
, frame(NULL)
#ifdef SAVE_YUV_DUMP
, video_dst_file(NULL)
#endif
, video_frame_count(0)
, audio_frame_count(0)
, dst_pix_fmt(AV_PIX_FMT_BGR24)
, dst_bufsize(0)
, sws_ctx(NULL)
{
	/* register all formats and codecs */
	av_register_all();
}

FlvReader::~FlvReader()
{

}

bool FlvReader::init(const std::string filename, scoreHandleCallBack pCallback)
{
	m_rgbCallback = pCallback;

	video_frame_count = 0;
	audio_frame_count = 0;

	/* open input file, and allocate format context */
	if (avformat_open_input(&fmt_ctx, filename.c_str(), NULL, NULL) < 0) {
		fprintf(stderr, "Could not open source file %s\n", filename.c_str());
	}

	/* retrieve stream information */
	if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
		fprintf(stderr, "Could not find stream information\n");
	}

	if (open_codec_context(&video_stream_idx, fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0) {
		video_stream = fmt_ctx->streams[video_stream_idx];
		video_dec_ctx = video_stream->codec;

#ifdef SAVE_YUV_DUMP
		video_dst_file = fopen(video_dst_filename.c_str(), "wb");
		if (!video_dst_file) {
			fprintf(stderr, "Could not open destination file %s\n", video_dst_filename.c_str());
			return false;
		}
#endif

		/* allocate image where the decoded image will be put */
		int ret = av_image_alloc(video_dst_data, video_dst_linesize,
			video_dec_ctx->width, video_dec_ctx->height,
			video_dec_ctx->pix_fmt, 1);
		if (ret < 0) {
			fprintf(stderr, "Could not allocate raw video buffer\n");
			return false;
		}

		if ((ret = av_image_alloc(dst_data, dst_linesize,
			video_dec_ctx->width, video_dec_ctx->height, dst_pix_fmt, 1)) < 0) {
			fprintf(stderr, "Could not allocate destination image\n");
			return false;
		}
		dst_bufsize = ret;

		video_dst_bufsize = ret;
	}

	if (open_codec_context(&audio_stream_idx, fmt_ctx, AVMEDIA_TYPE_AUDIO) >= 0) {
		audio_stream = fmt_ctx->streams[audio_stream_idx];
		audio_dec_ctx = audio_stream->codec;
#if 0
		audio_dst_file = fopen(audio_dst_filename, "wb");
		if (!audio_dst_file) {
			fprintf(stderr, "Could not open destination file %s\n", video_dst_filename);
			ret = 1;
			goto end;
		}
#endif
	}

	/* dump input information to stderr */
	av_dump_format(fmt_ctx, 0, filename.c_str(), 0);

	if (!audio_stream && !video_stream) {
		fprintf(stderr, "Could not find audio or video stream in the input, aborting\n");
		return false;
	}

	/* When using the new API, you need to use the libavutil/frame.h API, while
	* the classic frame management is available in libavcodec */
	frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Could not allocate frame\n");
		return false;
	}

	/* initialize packet, set data to NULL, let the demuxer fill it */
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;

	/* create scaling context */
	sws_ctx = sws_getContext(video_dec_ctx->width, video_dec_ctx->height, video_dec_ctx->pix_fmt,
		video_dec_ctx->width, video_dec_ctx->height, dst_pix_fmt,
		SWS_BILINEAR, NULL, NULL, NULL);
	if (!sws_ctx) {
		fprintf(stderr,
			"Impossible to create scale context for the conversion "
			"fmt:%s s:%dx%d -> fmt:%s s:%dx%d\n",
			av_get_pix_fmt_name(video_dec_ctx->pix_fmt), video_dec_ctx->width, video_dec_ctx->height,
			av_get_pix_fmt_name(dst_pix_fmt), video_dec_ctx->width, video_dec_ctx->height);
		return false;
	}
}

int FlvReader::open_codec_context(int *stream_idx,
	AVFormatContext *fmt_ctx, enum AVMediaType type)
{
	int ret;
	AVStream *st;
	AVCodecContext *dec_ctx = NULL;
	AVCodec *dec = NULL;
	AVDictionary *opts = NULL;

	ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
	if (ret < 0) {
		fprintf(stderr, "Could not find %s stream in input file\n",
			av_get_media_type_string(type));
		return ret;
	}
	else {
		*stream_idx = ret;
		st = fmt_ctx->streams[*stream_idx];

		/* find decoder for the stream */
		dec_ctx = st->codec;
		dec = avcodec_find_decoder(dec_ctx->codec_id);
		if (!dec) {
			fprintf(stderr, "Failed to find %s codec\n",
				av_get_media_type_string(type));
			return AVERROR(EINVAL);
		}

		/* Init the decoders, with or without reference counting */
		av_dict_set(&opts, "refcounted_frames", "1", 0);
		if ((ret = avcodec_open2(dec_ctx, dec, &opts)) < 0) {
			fprintf(stderr, "Failed to open %s codec\n",
				av_get_media_type_string(type));
			return ret;
		}
	}

	return 0;
}

bool FlvReader::readFrame()
{
	int got_frame;
	/* read frames from the file */
	while (av_read_frame(fmt_ctx, &pkt) >= 0) {
		AVPacket orig_pkt = pkt;
		do {
			int ret = decode_packet(&got_frame, 0);
			if (ret < 0)
				break;
			pkt.data += ret;
			pkt.size -= ret;
		} while (pkt.size > 0);
		av_free_packet(&orig_pkt);
	}

	/* flush cached frames */
	pkt.data = NULL;
	pkt.size = 0;
	do {
		decode_packet(&got_frame, 1);
	} while (got_frame);
}

char* FlvReader::avts2timestr(int64_t ts, AVRational *tb)
{
	char temp[AV_TS_MAX_STRING_SIZE] = {0};
	return av_ts_make_time_string(temp, ts, tb);
}

int FlvReader::decode_packet(int *got_frame, int cached)
{
	int ret = 0;
	int decoded = pkt.size;

	*got_frame = 0;

	if (pkt.stream_index == video_stream_idx) {
		/* decode video frame */
		ret = avcodec_decode_video2(video_dec_ctx, frame, got_frame, &pkt);
		if (ret < 0) {
			//fprintf(stderr, "Error decoding video frame (%s)\n", av_err2str(ret));
			return ret;
		}

		if (*got_frame) {
			uint32_t outPts = (uint32_t)(frame->pkt_pts);
			frame->pts = outPts;

			//printf("video_frame%s n:%d coded_n:%d pts:%s\n",
			//	cached ? "(cached)" : "",
			//	video_frame_count++, frame->coded_picture_number,
			//	avts2timestr(frame->pts, &video_dec_ctx->time_base));
#ifdef SAVE_YUV_DUMP
			//yuv data
			/* copy decoded frame to destination buffer:
			* this is required since rawvideo expects non aligned data */
			av_image_copy(video_dst_data, video_dst_linesize,
				(const uint8_t **)(frame->data), frame->linesize,
				video_dec_ctx->pix_fmt, video_dec_ctx->width, video_dec_ctx->height);

			/* write to rawvideo file */
			fwrite(video_dst_data[0], 1, video_dst_bufsize, video_dst_file);
#else
			sws_scale(sws_ctx, (const uint8_t **)(frame->data),
				frame->linesize, 0, video_dec_ctx->height, dst_data, dst_linesize);

			/* write to rawvideo file */
			//fwrite(dst_data[0], 1, dst_bufsize, video_dst_file);

			m_rgbCallback(dst_data[0], video_dec_ctx->width, video_dec_ctx->height, frame->pts);
#endif
		}
	}
	else if (pkt.stream_index == audio_stream_idx) {
		/* decode audio frame */
		ret = avcodec_decode_audio4(audio_dec_ctx, frame, got_frame, &pkt);
		if (ret < 0) {
			//fprintf(stderr, "Error decoding audio frame (%s)\n", av_err2str(ret));
			return ret;
		}
		/* Some audio decoders decode only part of the packet, and have to be
		* called again with the remainder of the packet data.
		* Sample: fate-suite/lossless-audio/luckynight-partial.shn
		* Also, some decoders might over-read the packet. */
		decoded = FFMIN(ret, pkt.size);

		if (*got_frame) {
#if 0
			size_t unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample(frame->format);
			printf("audio_frame%s n:%d nb_samples:%d pts:%s\n",
				cached ? "(cached)" : "",
				audio_frame_count++, frame->nb_samples,
				av_ts2timestr(frame->pts, &audio_dec_ctx->time_base));

			/* Write the raw audio data samples of the first plane. This works
			* fine for packed formats (e.g. AV_SAMPLE_FMT_S16). However,
			* most audio decoders output planar audio, which uses a separate
			* plane of audio samples for each channel (e.g. AV_SAMPLE_FMT_S16P).
			* In other words, this code will write only the first audio channel
			* in these cases.
			* You should use libswresample or libavfilter to convert the frame
			* to packed data. */
			fwrite(frame->extended_data[0], 1, unpadded_linesize, audio_dst_file);
#endif
		}
	}
#if 0
	/* If we use the new API with reference counting, we own the data and need
	* to de-reference it when we don't use it anymore */
	if (*got_frame && api_mode == API_MODE_NEW_API_REF_COUNT)
		av_frame_unref(frame);
#endif
	return decoded;
}

void FlvReader::deInit()
{
	avcodec_close(video_dec_ctx);
	avcodec_close(audio_dec_ctx);
	avformat_close_input(&fmt_ctx);
	av_frame_free(&frame);

#ifdef SAVE_YUV_DUMP
	if (video_dst_file)
		fclose(video_dst_file);
#endif

	av_freep(&dst_data[0]);
	sws_freeContext(sws_ctx);
}