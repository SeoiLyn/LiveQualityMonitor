#ifndef __FLV_READER_H__
#define __FLV_READER_H__

#include <string>

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

typedef void(*scoreHandleCallBack)(uint8_t * frameData, int width, int height, int64_t pts);//statusCode 1为正确，-1为错误

//#define SAVE_YUV_DUMP

class FlvReader
{
public:
	FlvReader();
	virtual ~FlvReader();

public:
	bool init(const std::string filename, scoreHandleCallBack pCallback);
	bool readFrame();
	void deInit();

private:
	int open_codec_context(int *stream_idx,
		AVFormatContext *fmt_ctx, enum AVMediaType type);
	int decode_packet(int *got_frame, int cached);
	char* avts2timestr(int64_t ts, AVRational *tb);

private:
	AVFormatContext *fmt_ctx;
	AVCodecContext *video_dec_ctx;
	AVCodecContext *audio_dec_ctx;
	AVStream *video_stream;
	AVStream *audio_stream;
	int video_stream_idx;
	int audio_stream_idx;
	AVFrame *frame;
	AVPacket pkt;
	int video_frame_count;
	int audio_frame_count;
	AVPixelFormat dst_pix_fmt;
	scoreHandleCallBack m_rgbCallback;

private:
	//temp
	uint8_t *video_dst_data[4];
	int      video_dst_linesize[4];
	int      video_dst_bufsize;
	static std::string video_dst_filename;
#ifdef SAVE_YUV_DUMP
	FILE *video_dst_file;
#endif

private:
	uint8_t *dst_data[4];
	int dst_linesize[4];
	int dst_bufsize;
	struct SwsContext *sws_ctx;
};

#endif /* __FLV_READER_H__ */
