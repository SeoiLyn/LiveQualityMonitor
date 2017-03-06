/*
 * SaveFlv.cpp
 *
 *  Created on: 2016年8月7日
 *      Author: Administrator
 */

#include <sstream>
#include "SaveFlv.h"
#define FUNLOG_WITH_LINE(level, fmt, ...) printf("[%s:%d]SaveFlv " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

namespace server
{
namespace dir
{

SaveFlv::SaveFlv(const uint32_t pid, const std::string& type, const int id):
		m_pAvOutfmtctx(NULL), m_strWrapperType(type), m_iWrapperId(id), m_uPid(pid)
{
	/*
	std::stringstream ss;
	ss << Objs::Instance()->m_pConf->GetFilePath() << "/flv/";
	std::string dir = ss.str();
	if(!Directory::IsDirectory(dir))
	{
		if(!Directory::CreateDirectory(dir))
		{
			//log
		}
	}*/
	av_register_all();
	//log
}

SaveFlv::~SaveFlv()
{
	Exit();
}

int SaveFlv::ReInit(const AVFormatContext* pfmtctx, const AVDictionary *opts)
{
	Exit();
	return Init(pfmtctx, opts);
}

int SaveFlv::Init(const AVFormatContext* pfmtctx, const AVDictionary *opts)
{
	int ret = 0;
	std::stringstream ss;
	ss << "~/temp/flv/";
	//int curTime = time(NULL);
	char time_buf[30];
	time_t cur = time(NULL);
	strftime(time_buf, 30, "%Y%m%d%H%M%S", localtime(&cur));
	ss << m_uPid << "_" << m_strWrapperType << "_"
			<< m_iWrapperId << "_" << time_buf << ".flv";
	m_strFileName = ss.str();
	//log
	avformat_alloc_output_context2(&m_pAvOutfmtctx, 0, "flv",
			m_strFileName.c_str());
	if (!m_pAvOutfmtctx)
	{
		FUNLOG_WITH_LINE(Error, "Could not create output(%s) context\n", m_strFileName.c_str());
		return AVERROR_UNKNOWN;
	}
	AVOutputFormat *ofmt = m_pAvOutfmtctx->oformat;
	for (size_t i = 0; i < pfmtctx->nb_streams; i++)
	{
		//根据输入流创建输出流（Create output AVStream according to input AVStream）
		AVStream *in_stream = pfmtctx->streams[i];
		AVStream *out_stream = NULL;
		out_stream = avformat_new_stream(m_pAvOutfmtctx,
				in_stream->codec->codec);
		if (!out_stream)
		{
			FUNLOG_WITH_LINE(Error, "Failed allocating output stream\n");
			ret = AVERROR_UNKNOWN;
			break;
		}
		else
		{
			//复制AVCodecContext的设置（Copy the settings of AVCodecContext）
			ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
			if (ret < 0)
			{
				FUNLOG_WITH_LINE(Error, "Failed to copy context from input to output stream codec context\n");
				ret = AVERROR_UNKNOWN;
				break;
			}
			else
			{
				out_stream->codec->codec_tag = 0;
				if (m_pAvOutfmtctx->oformat->flags & AVFMT_GLOBALHEADER)
					out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
			}
		}
	}

	if(!ret)
	{
		av_dump_format(m_pAvOutfmtctx, 0, m_strFileName.c_str(), 1);
		if (!(ofmt->flags & AVFMT_NOFILE)) {
			ret = avio_open(&m_pAvOutfmtctx->pb, m_strFileName.c_str(), AVIO_FLAG_WRITE);
			if (ret >= 0) 
			{
				ret = avformat_write_header(m_pAvOutfmtctx, NULL);
			}
			else
			{
				FUNLOG_WITH_LINE(Error, "avio_open fail.\n");
			}
		}
	}
	if (ret)
	{
		if (m_pAvOutfmtctx && !(ofmt->flags & AVFMT_NOFILE))
			avio_close(m_pAvOutfmtctx->pb);
		avformat_free_context(m_pAvOutfmtctx);
		m_pAvOutfmtctx = NULL;
	}

	return ret;
}

int SaveFlv::WriteFrameToFlv(const AVPacket* pAvPkt)
{
#if 0
	//log
	AVPacket* pPktClone = av_packet_clone(pAvPkt);
	int ret = av_interleaved_write_frame(m_pAvOutfmtctx, pPktClone);
	return ret;
#else
	return 0;
#endif
}

void SaveFlv::Exit()
{
	if(m_pAvOutfmtctx)
	{
		av_write_trailer(m_pAvOutfmtctx);
		if (m_pAvOutfmtctx->oformat && !(m_pAvOutfmtctx->oformat->flags & AVFMT_NOFILE))
			avio_close(m_pAvOutfmtctx->pb);
		avformat_free_context(m_pAvOutfmtctx);
		m_pAvOutfmtctx = NULL;
	}
}

} /* namespace dir */
} /* namespace server */
