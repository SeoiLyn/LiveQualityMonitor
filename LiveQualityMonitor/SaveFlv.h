/*
 * SaveFlv.h
 *
 *  Created on: 2016年8月7日
 *      Author: Administrator
 */

#ifndef _SAVE_FLV_H_
#define _SAVE_FLV_H_
#include <string>

extern "C" {
#include "libavformat/avformat.h"
}

namespace server
{
namespace dir
{

class SaveFlv
{
public:
	SaveFlv(const uint32_t pid, const std::string& type, const int id);
	virtual ~SaveFlv();
public:
	int Init(const AVFormatContext* pfmtctx, const AVDictionary *opts);
	int ReInit(const AVFormatContext* pfmtctx, const AVDictionary *opts);
	int WriteFrameToFlv(const AVPacket* pAvPkt);
	void Exit();
private:
	AVFormatContext *m_pAvOutfmtctx;
	std::string m_strWrapperType;
	int m_iWrapperId;
	uint32_t m_uPid;
	std::string m_strFileName;
};

} /* namespace dir */
} /* namespace server */

#endif /* _SAVE_FLV_H_ */
