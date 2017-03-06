#include <cstdio>
#include "SaveFlv.h"
#include <stdio.h>
#include "librtmp/rtmp.h"
#include "librtmp/log.h"
#include "FlvReader.h"
#include "brisque_revised/brisque.h"
#include <string>
#include <iostream>
#include "Inspector.h"

//float rescale_vector[36][2];
//std::string prefix = "/home/water/projects/LiveQualityMonitor/brisque_revised/";

void log(char filename[], char visitor[])
{
	FILE * pLog;
	pLog = fopen(filename, "a");
	if (pLog != NULL)
	{
		fputs(visitor, pLog);
		fputs("\n", pLog);
		fclose(pLog);
	}
}

int saveRtmp()
{
	double duration = -1;
	int nRead;
	//is live stream ?
	bool bLiveStream = true;


	int bufsize = 1024 * 1024 * 10;
	char *buf = (char*)malloc(bufsize);
	memset(buf, 0, bufsize);
	long countbufsize = 0;

	FILE *fp = fopen("/home/water/temp/receive.flv", "wb");
	if (!fp) {
		RTMP_LogPrintf("Open File Error.\n");
		return -1;
	}

	/* set log level */
	//RTMP_LogLevel loglvl=RTMP_LOGDEBUG;
	//RTMP_LogSetLevel(loglvl);

	RTMP *rtmp = RTMP_Alloc();
	RTMP_Init(rtmp);
	//set connection timeout,default 30s
	rtmp->Link.timeout = 10;

	if (!RTMP_SetupURL(rtmp, "rtmp://172.25.35.13/live/test"))
	{
		RTMP_Log(RTMP_LOGERROR, "SetupURL Err\n");
		RTMP_Free(rtmp);
		return -1;
	}
	if (bLiveStream) {
		rtmp->Link.lFlags |= RTMP_LF_LIVE;
	}

	//1hour
	RTMP_SetBufferMS(rtmp, 3600 * 1000);

	if (!RTMP_Connect(rtmp, NULL)) {
		RTMP_Log(RTMP_LOGERROR, "Connect Err\n");
		RTMP_Free(rtmp);
		return -1;
	}

	if (!RTMP_ConnectStream(rtmp, 0)) {
		RTMP_Log(RTMP_LOGERROR, "ConnectStream Err\n");
		RTMP_Free(rtmp);
		RTMP_Close(rtmp);
		return -1;
	}

#if 1
	while (nRead = RTMP_Read(rtmp, buf, bufsize)) {
		fwrite(buf, 1, nRead, fp);

		countbufsize += nRead;
		RTMP_LogPrintf("Receive: %5dByte, Total: %5.2fkB\n", nRead, countbufsize*1.0 / 1024);
	}
#else
	RTMPPacket *packet = NULL;
	RTMPPacket ps = { 0 };

	while (nRead = RTMP_GetNextMediaPacket(rtmp, &ps)) {
		RTMP_LogPrintf("Receive: %5dByte\n", nRead);
	}
	//RTMPPacket_Free(packet);
#endif

	if (fp)
		fclose(fp);

	if (buf) {
		free(buf);
	}

	if (rtmp) {
		RTMP_Close(rtmp);
		RTMP_Free(rtmp);
		rtmp = NULL;
	}
}

Inspector inspector;

void computeRGBScore(uint8_t * frameData, int width, int height, int64_t pts)
{
	inspector.computeRGBScore(frameData, width, height, pts);
}

/*
@param1 flv url address
@param2 quality record filename
 */
int main(int argc, char* argv[])
{
	//saveRtmp();
#if 1

#if 1
	char* flv_url = argv[1];
#else
	//char* flv_url = "/home/water/temp/receive.flv";//read from file directly
	char* flv_url = "http://hls.yy.com/1337827894_1337827894_15013.flv?uuid=123321";
#endif

#if 1
	char* record_file = argv[2];
#else
	char* record_file = "/home/water/temp/receive3.csv";
#endif

	//read in the allrange file to setup internal scaling array
	if (Inspector::read_range_file(Inspector::prefix)) {
		cerr << "unable to open allrange file" << endl;
		return -1;
    }

	inspector.init(record_file);

	FlvReader reader;
	reader.init(flv_url, computeRGBScore);
	reader.readFrame();
	reader.deInit();

	inspector.unInit();
#else
	float qualityscore;

	//read in the allrange file to setup internal scaling array
	if (Inspector::read_range_file(Inspector::prefix)) {
		cerr << "unable to open allrange file" << endl;
		return -1;
	}

	int istrain = 1;
	if (!istrain) //default value is 1 for false?
		trainModel();

	char* filename = "/home/water/temp/images/testimage2.bmp";
	qualityscore = computescore(Inspector::prefix, filename);
	std::cout << "score in main file is given by:" << qualityscore << std::endl;
#endif

	return 0;
}