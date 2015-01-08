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
}SGN_AssetMgr;

typedef enum {
	eNewPlaylist,
	eMakeHeadReq,
	eMakeDwldReq,
	eCheckSpace,
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
	PlaylistData *pAd;
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

int makeRequest(SGN_AssetMgr *pAssetMgr, sgn_list_t *pCurrent, sgn_bool isHead) {
	SGNCB_DwldComplete cbComplete = NULL;
	if(NULL == pAssetMgr || NULL == pCurrent) {
		return -1;
	}
	logPrint(pAssetMgr->hLogHandle, LogLevelInfo, "Entering makeRequest");
	PlaylistData *pAd	= (PlaylistData *)pCurrent->data;
	char *pAssetName	= sgn_get_name_from_url(pAd->pUrl);
	if(isHead && isAssetAvailable(pAssetName)) {
		logPrint(pAssetMgr->hLogHandle, LogLevelInfo, "Asset: %s is available in cache", pAssetName);
		return 0;
	}

	if(isHead) {
		logPrint(pAssetMgr->hLogHandle, LogLevelInfo, "Making head request for URL: %s", pAd->pUrl);
		cbComplete	= headReqComplete;
	} else {
		logPrint(pAssetMgr->hLogHandle, LogLevelInfo, "Making download request for URL: %s", pAd->pUrl);
		cbComplete	= dwlReqComplete;
	}
	AssetDwldReq *pAssetReq	= calloc(sizeof(SGN_AssetReq), 1);
	pAssetReq->iReqId		= sgn_get_unique_no();
	pAssetReq->pNextAd		= pCurrent->next;
	pAssetReq->pAssetMgr	= pAssetMgr;
	pAssetReq->pAd			= pCurrent->data;
	pAssetMgr->iReqCount++;
	pAssetMgr->pDwldReqQ	= sgn_list_append(pAssetMgr->pDwldReqQ, pAssetReq);
	sgn_bool isFile			= (sgn_false == isHead);
	startDownload(pAssetMgr->hDwldHandle, pAd->pUrl, pAssetMgr->pDwldReqQ, NULL, isHead, isFile, cbComplete, pAssetReq->iReqId);
	logPrint(pAssetMgr->hLogHandle, LogLevelInfo, "Exiting makeRequest");
	return 0;
}

int initiateHeadRequests(SGN_AssetMgr *pAssetMgr, sgn_list_t *pPlaylist) {
	logPrint(pAssetMgr->hLogHandle, LogLevelInfo, "Entering initiateHeadRequests");
	sgn_list_t *pList	= sgn_list_first(pPlaylist);
	makeRequest(pAssetMgr, pList, sgn_true);
	logPrint(pAssetMgr->hLogHandle, LogLevelInfo, "Exiting initiateHeadRequests");
	return 0;
}

int initiateDownloadRequests(SGN_AssetMgr *pAssetMgr) {
	logPrint(pAssetMgr->hLogHandle, LogLevelInfo, "Entering initiateDownloadRequests");
	sgn_list_t *pList	= sgn_list_first(pPlaylist);
	makeRequest(pAssetMgr, pList, sgn_false);
	logPrint(pAssetMgr->hLogHandle, LogLevelInfo, "Exiting initiateDownloadRequests");
	return 0;
}

void checkSpaceForAssets(SGN_AssetMgr *pAssetMgr) {
	//	if space not available, make request to recover
	//	otherwise, make download request

	//	Now we are assuming enough space is there and proceeding to make download request
	AssetThrdTask *pReq		= calloc(sizeof(AssetMgrDwldReq), 1);
	pReq->iReqMsg			= eMakeDwldReq;
	pReq->pData				= pAssetMgr;
}

void headReqComplete(int status, void *pHeadSize, void *pUserData, int iReqId) {
	if(NULL == pUserData || 0 >= iReqId) {
		return;
	}
	sgn_list_t *pDwldReqQ	= (sgn_list_t *)pUserData;
	AssetDwldReq *pAssetReq = (AssetDwldReq *)getDwldReqFromId(pDwldReqQ, iReqId);
	SGN_AssetMgr *pAssetMgr = (SGN_AssetMgr *)pAssetReq->pAssetMgr;
	logPrint(pAssetMgr->hLogHandle, LogLevelInfo, "Entering headReqComplete for URL: %s", pAssetReq->pAd->pUrl);

	if(pAssetReq->isReqTimedOut || DOWNLOAD_NO_ERROR != status) {
		logPrint(pAssetMgr->hLogHandle, LogLevelFatal, "Head request timedout");
		pAssetMgr->pAssetDwldComple(sgn_false, pAssetMgr->pUserData);
		return;
	}
	unsigned int iHeadSize		= 0;
	memcpy((void *)&iHeadSize, pHeadOrBuf, sizeof(unsigned int));
	pAssetReq->pAd->assetSize	= iHeadSize;
	pAssetMgr->iSpaceReqd		+= iHeadSize;
	pAssetMgr->iReqCount--;
	if(0 >= pAssetMgr->iReqCount) {
		//	check if space available for all
		AssetThrdTask *pReq		= calloc(sizeof(AssetMgrDwldReq), 1);
		pReq->iReqMsg			= eCheckSpace;
		pReq->pData				= pAssetMgr;
		sgn_async_queue_push(pAssetMgr->pAssetReqQ, pReq);
	} else {
		//	make next head request
		makeRequest(hAssetMgr, pAssetReq->pNextAd, sgn_true);
	}
}

void dwlReqComplete(int status, void *pBuffer, void *pUserData, int iReqId) {
	if(NULL == pUserData || 0 >= iReqId) {
		return;
	}
	sgn_list_t *pDwldReqQ	= (sgn_list_t *)pUserData;
	AssetDwldReq *pAssetReq = (AssetDwldReq *)getDwldReqFromId(pDwldReqQ, iReqId);
	SGN_AssetMgr *pAssetMgr = (SGN_AssetMgr *)pAssetReq->pAssetMgr;
	logPrint(pAssetMgr->hLogHandle, LogLevelInfo, "Entering dwlReqComplete for URL: %s", pAssetReq->pAd->pUrl);
	pAssetMgr->iReqCount--;
	if(0 >= pAssetMgr->iReqCount) {
		logPrint(pAssetMgr->hLogHandle, LogLevelInfo, "Asset download successfully complete");
		pAssetMgr->pAssetDwldComple(sgn_true, pAssetMgr->pUserData);
	} else {
		makeRequest(hAssetMgr, pAssetReq->pNextAd, sgn_false);
	}
	logPrint(pAssetMgr->hLogHandle, LogLevelInfo, "Exiting dwlReqComplete");
}

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
