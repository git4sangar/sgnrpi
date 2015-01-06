//gn DigitalSignage.cpp

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "DigitalSignage.h"
#include "SGNDownloader.h"
#include "SGNParser.h"
#include "SGNLogger.h"
#include "sgn_timer.h"


void onDwldComplete(int status, int iReqId, sgn_bool isHead,  void *pData);
void onTimerExpiry(SGN_TimerHandle pHandle, void *pUserData);

int main(void) {
	SGN_LogHandle hLogHandle	= initLogger(MAX_LOG_FILE_SIZE_MB);
	SGN_DwldHandle hDwldHandle	= initDownloader(hLogHandle);

	//char *pURL = "http://shadowcdn-01.yumenetworks.com/ym/229/suSCSNRo_384K_480x360.mp4";
	char *pURL = "http://www.madhangi.com/dynamic_preroll_playlist.vast2xml";
	startDownload(hDwldHandle, pURL, "shampoo.mp4", NULL, sgn_false, sgn_false, onDwldComplete, 10);

	/*FILE *fp = fopen("/home/sangar/proj/rpi/dsgn/dynamic_playlist.json", "r");
	char jsonPkt[2048];
	int iRet = fread(jsonPkt, 1, 2048, fp);
	fclose(fp);

	jsonPkt[iRet] = '\0';
	printf("json packet: %s\n", jsonPkt);
	parsePlaylist(jsonPkt, hLogHandle);*/

	while(1) {
		sleep(1);
	}
	return 0;
}

void onDwldComplete(int status, int iReqId, sgn_bool isHead,  void *pData) {
	unsigned int headSize = 0;
	char *pXml = NULL;

	if(NULL != pData) {
		if(isHead) {
			memcpy((void *)&headSize, pData, sizeof(unsigned int));
		} else {
			pXml = pData;
		}
	}
	printf("Download status: %d, iReqId: %d, headSize: %d\n", status, iReqId, headSize);
	printf("XML file is \n%s\n\n\n", pXml);
	if(pData) free(pData);
}

void onTimerExpiry(SGN_TimerHandle pHandle, void *pUserData) {
	printf("Timer expiry\n");
	sgn_addto_timer(pHandle, 1, NULL);
}
