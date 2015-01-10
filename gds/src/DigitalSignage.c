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


void onDwldComplete(int status, void *pHeadOrBuf, void *pUserData, int iReqId);
void assetDwldComplete(sgn_bool status, void *pUserData);
void onTimerExpiry(SGN_TimerHandle pHandle, void *pUserData);

int main(void) {
	SGN_Handle *hSgnMgr		= calloc(sizeof(SGN_Handle), 1);
	hSgnMgr->hLogHandle		= initLogger(MAX_LOG_FILE_SIZE_MB);
	hSgnMgr->hDownloader	= initDownloader(hSgnMgr->hLogHandle);
	hSgnMgr->hAssetMgr		= initAssetManager(hSgnMgr->hLogHandle, hSgnMgr->hDownloader, assetDwldComplete, hSgnMgr);
	char *pURL = "http://www.madhangi.com/sgn/dynamic_playlist.json";
	startDownload(hSgnMgr->hDownloader, pURL, hSgnMgr, NULL, sgn_false, sgn_false, onDwldComplete, 1);
	//char *pURL = "http://shadowcdn-01.yumenetworks.com/ym/229/suSCSNRo_384K_480x360.mp4";
	//char *pURL = "http://www.madhangi.com/dynamic_preroll_playlist.vast2xml";

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

void assetDwldComplete(sgn_bool status, void *pUserData) {}

void onDwldComplete(int status, void *pHeadOrBuf, void *pUserData, int iReqId) {
	SGN_Handle *hSgnMgr	= (SGN_Handle *)pUserData;
	sgn_list_t *pAdList = parsePlaylist(pHeadOrBuf, hSgnMgr->hLogHandle);
	downloadPlaylistData(hSgnMgr->hAssetMgr, pAdList);
}

void onTimerExpiry(SGN_TimerHandle pHandle, void *pUserData) {
	printf("Timer expiry\n");
	sgn_addto_timer(pHandle, 1, NULL);
}
