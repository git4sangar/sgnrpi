/*
 * SGNAssetMgr.c
 *
 *  Created on: 02-Jan-2015
 *      Author: sangar
 */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "SGNAssetMgr.h"
#include "SGNParser.h"
#include "SGNDownloader.h"
#include "SGNLogger.h"
#include "sgn_list.h"
#include "sgn_queue.h"
#include "sgn_string.h"
#include "sgn_utils.h"

typedef struct {
	SGN_LogHandle hLogHandle;
	SGN_DwldHandle hDwldHandle;
	SGNCB_AssetDwldComplete pAssetDwldComplete;
	pthread_mutex_t assetMgrMtx;
	pthread_t assetDwldThread;
	sgn_async_queue_t *pAssetReqQ;
	sgn_list_t *pPlaylist;
	sgn_list_t *pDwldReqQ;
	void *pUserData;
	int iReqCount;
	int iFreeSpaceMB;
	int iSpaceReqd;
	sgn_bool isDwldSucceeded;
}SGN_AssetMgr;

typedef enum {
	eNewPlaylist,
	eMakeHeadReq,
	eMakeDwldReq,
	eRecoverSpace
}AssetMgrReqMsg;

typedef struct {
	AssetMgrReqMsg iReqMsg;
	void *pData;
}AssetThrdTask;

typedef struct {
	int iReqId;
	sgn_bool isReqTimedOut;
	SGN_AssetMgr *pAssetMgr;
	sgn_list *pNextAd;
}AssetDwldReq;

SGN_AssetMgrHandle initAssetManager(SGN_LogHandle hLogger, SGN_DwldHandle hDownloader,
										SGNCB_AssetDwldComplete pAssetDwldComplete, void *pUserData) {
	if(hLogger) {
		return NULL;
	}
	logPrint(hLogger, LogLevelInfo, "Entering Asset Manager initialization");
	if(NULL == hDownloader || NULL == pAssetDwldComplete) {
		logPrint(hLogger, LogLevelError, "Invalid input to asset manager initialization");
		return NULL;
	}

	SGN_AssetMgr *pAssetMgr			= calloc(sizeof(SGN_AssetMgr), 1);
	pAssetMgr->hDwldHandle			= hDownloader;
	pAssetMgr->hLogHandle			= hLogger;
	pAssetMgr->pAssetDwldComplete	= pAssetDwldComplete;
	pAssetMgr->pUserData			= pUserData;

	pAssetMgr->pAssetReqQ = sgn_async_queue_new();
	if(NULL == pAssetMgr->pAssetReqQ) {
		logPrint(hLogger, LogLevelError, "Error creating async Q asset downloader");
		return NULL;
	}

	if( 0 != pthread_mutex_init(&pAssetMgr->assetMgrMtx, NULL) ) {
		logPrint(hLogger, LogLevelError, "Error creating mutex for asset downloader");
		return NULL;
	}

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_create(&pAssetMgr->assetDwldThread, &attr, assetDownloadThread, pAssetMgr);
	pthread_attr_destroy(&attr);

	logPrint(hLogger, LogLevelInfo, "Exiting Asset Manager initialization");
	return (SGN_AssetMgrHandle)pAssetMgr;
}

int downloadPlaylistData(SGN_AssetMgrHandle hAssetMgr, sgn_list_t *pPlaylist) {
	if(NULL == hAssetMgr || NULL == pPlaylist) {
		return -1;
	}
	SGN_AssetMgr *pAssetMgr = (SGN_AssetMgr *)hAssetMgr;
	logPrint(pAssetMgr->hLogHandle, LogLevelInfo, "Entering downloadPlaylistData");
	AssetThrdTask *pReq		= calloc(sizeof(AssetMgrDwldReq), 1);
	pReq->iReqMsg			= eNewPlaylist;
	pAssetMgr->pPlaylist	= pPlaylist;
	pAssetMgr->iReqCount	= 0;
	sgn_async_queue_push(pAssetMgr->pAssetReqQ, pReq);
	return 0;
}

int makeHeadRequest(SGN_AssetMgr *pAssetMgr, sgn_list_t *pCurrent) {
	if(NULL == pAssetMgr || NULL == pCurrent) {
		return -1;
	}

	logPrint(pAssetMgr->hLogHandle, LogLevelInfo, "Entering makeHeadRequest for URL: %s");
	PlaylistData *pAd	= (PlaylistData *)pCurrent->data;
	char *pAssetName	= sgn_get_name_from_url(pAd->pUrl);
	if(!isAssetAvailable(pAssetName)) {
		logPrint(pAssetMgr->hLogHandle, LogLevelInfo, "Asset: %s is not available in cache", pAssetName);
		AssetDwldReq *pAssetReq	= calloc(sizeof(SGN_AssetReq), 1);
		pAssetReq->iReqId		= sgn_get_unique_no();
		pAssetReq->pNextAd		= pCurrent->next;
		pAssetReq->pAssetMgr	= pAssetMgr;
		pAssetMgr->iReqCount++;
		pAssetMgr->pDwldReqQ	= sgn_list_append(pAssetMgr->pDwldReqQ, pAssetReq);
		startDownload(pAssetMgr->hDwldHandle, pAd->pUrl, pAssetMgr->pDwldReqQ, NULL,
						sgn_true, sgn_false, headReqComplete, pAssetReq->iReqId);
	} else {
		logPrint(pAssetMgr->hLogHandle, LogLevelInfo, "Asset: %s is available in cache", pAssetName);
		return -1;
	}
	return 0;
}

int initiateHeadRequests(SGN_AssetMgrHandle hAssetMgr, sgn_list_t *pPlaylist) {
	if(NULL == hAssetMgr) {
		return -1;
	}

	SGN_AssetMgr *pAssetMgr = (SGN_AssetMgr *)hAssetMgr;
	pAssetMgr->isDwldSucceeded = sgn_true;	// will be set to false when download fails
	logPrint(pAssetMgr->hLogHandle, LogLevelInfo, "Entering initiateHeadRequests");
	if(NULL == pPlaylist) {
		logPrint(pAssetMgr->hLogHandle, LogLevelError, "Invalid params to startAssetDownload");
		return -1;
	}

	sgn_list_t *pList	= sgn_list_first(pPlaylist);
	makeHeadRequest(pAssetMgr, pList);
	return 0;
}

void headReqComplete(int status, void *pHeadOrBuf, void *pUserData, int iReqId) {
	if(NULL == pUserData || 0 >= iReqId) {
		return;
	}
	sgn_list_t *pDwldReqQ	= (sgn_list_t *)pUserData;
	AssetDwldReq *pAssetReq = (AssetDwldReq *)getDwldReqFromId(pDwldReqQ, iReqId);
	SGN_AssetMgr *pAssetMgr = (SGN_AssetMgr *)pAssetReq->pAssetMgr;
	logPrint(pAssetMgr->hLogHandle, LogLevelInfo, "Entering headReqComplete");

	if(pAssetReq->isReqTimedOut) {
		logPrint(pAssetMgr->hLogHandle, LogLevelFatal, "Head request timedout");
		pAssetMgr->pAssetDwldComple(sgn_false, pAssetMgr->pUserData);
	}
	pAssetMgr->isDwldSucceeded = pAssetMgr->isDwldSucceeded && (status == DOWNLOAD_NO_ERROR);
	pAssetMgr->iReqCount--;
	unsigned int iHeadSize = 0;
	memcpy((void *)&iHeadSize, pHeadOrBuf, sizeof(unsigned int));
	pAssetMgr->iSpaceReqd	+= iHeadSize;
	if(0 >= pAssetMgr->iReqCount) {
		//	check if space available for all
		//	make asset download request
	} else {
		//	make next head request
		makeHeadRequest(hAssetMgr, pAssetReq->pNextAd);
	}
}

void postReqComplete(int status, int iReqId, sgn_bool isHead, void *pData){}

int loadAssetList(SGN_AssetMgrHandle hAssetMgr, char *pDirWithSlash, char *pFileName) {
	if(NULL == hAssetMgr) {
		return -1;
	}
	SGN_AssetMgr *pAssetMgr = (SGN_AssetMgr *)hAssetMgr;

	//	Sanity check
	logPrint(pAssetMgr->hLogHandle, LogLevelInfo, "Entering loadAssetList");
	if(NULL == pDirWithSlash || NULL == pFileName) {
		logPrint(pAssetMgr->hLogHandle, LogLevelError, "Invalid input to loadAssetList");
		return -1;
	}

	//	Create the directory if does not exists
	int iRet = access(pDirWithSlash, F_OK);
	if(0 != iRet) {
		if (ENOENT == errno) {
			logPrint(pAssetMgr->hLogHandle, LogLevelWarn, "Directory does not exist");
		}
		if(ENOTDIR == errno) {
			logPrint(pAssetMgr->hLogHandle, LogLevelError, "It is not a directory. So deleting");
			ulink(pDirWithSlash);
		}
		iRet = mkdir(pDirWithSlash, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		if(0 != iRet) {
			logPrint(pAssetMgr->hLogHandle, LogLevelFatal, "Directory could not be created");
			return -1;
		}
	}

	//	Open the asset file. Create if does not exists
	int iLen = sgn_strlen(pDirWithSlash) + sgn_strlen(pFileName) + 1;
	char *pAssetFile	= calloc(iLen, 1);
	strcpy(pAssetFile, pDirWithSlash);
	strcat(pAssetFile, pFileName);
	FILE *fp = fopen(pAssetFile, "r");
	if(NULL == fp) {
		logPrint(pAssetMgr->hLogHandle, LogLevelWarn, "Asset list file does not exist");
		fp = fopen(pAssetFile, "w");
	}
	iRet = fread();
	return 0;
}

void *assetDownloadThread(void *pData) {
	if(NULL == pData) {
		return NULL;
	}
	SGN_AssetMgr *pAssetMgr	= pData;
	logPrint(pAssetMgr->hLogHandle, LogLevelInfo, "Entering asset download thread");

	while(1) {
		SGN_AssetReq *pDwlReq	= sgn_async_queue_pop(pDwldHandle->pDwlReqQ);
	}
	return NULL;
}
