//gn

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <curl/curl.h>
#include <string.h>
#include "SGNDownloader.h"
#include "sgn_utils.h"
#include "sgn_queue.h"
#include "sgn_types.h"
#include "sgn_string.h"
#include "SGNLogger.h"

typedef struct {
	SGN_LogHandle pLogHandle;
	pthread_mutex_t dwldMtx;
	pthread_t threadDwldr;
	sgn_async_queue_t *pDwlReqQ;
	CURL *pCurl;
}SGN_Downloader;

typedef struct {
	char *pUrl;
	sgn_bool isHead;	//	ture if head request
	sgn_bool isFile;	//	tree while response needs to be written to file, otherwise to buffer
	sgn_bool isQuit;
	char *pPayload;
	unsigned int uiSizeFromHead;
	int iReqId;
	FILE *pAssetFile;
	void *pUserData;
	SGN_Downloader *pDwldHandle;
	SGNCB_DwldComplete pDwldComplete;
	sgn_string_t *pRespString;
}SGN_DwldReq;

void freeDownloadReq(SGN_DwldReq *pDwlReq) {
	free(pDwlReq->pUrl);
	free(pDwlReq->pPayload);
}

SGN_DwldHandle initDownloader(SGN_LogHandle logHandle) {
	SGN_Downloader *pDwldHandle = NULL;
	int iRet = -1;

	if(NULL == logHandle) {
		return NULL;
	}

	logPrint(logHandle, LogLevelInfo, "Entering Downloader initialization");

	pDwldHandle = calloc(sizeof(SGN_Downloader), 1);
	pDwldHandle->pLogHandle		= logHandle;
	if( 0 != pthread_mutex_init(&pDwldHandle->dwldMtx, NULL) ) {
		logPrint(logHandle, LogLevelError, "Error creating mutex for downloader module");
		return NULL;
	}
	pDwldHandle->pDwlReqQ = sgn_async_queue_new();
	if(NULL == pDwldHandle->pDwlReqQ) {
		logPrint(logHandle, LogLevelError, "Error creating async Q downloader module");
		return NULL;
	}

	//	Create curl handle
	pDwldHandle->pCurl = curl_easy_init();
	if(NULL == pDwldHandle->pCurl) {
		logPrint(logHandle, LogLevelError, "Error initializing curl");
		return NULL;
	}
	curl_easy_setopt(pDwldHandle->pCurl, CURLOPT_NOSIGNAL, 1);			// tell multi thread app
	curl_easy_setopt(pDwldHandle->pCurl, CURLOPT_FOLLOWLOCATION, 1);	// automatic redirection
	curl_easy_setopt(pDwldHandle->pCurl, CURLOPT_MAXREDIRS, 5);			// max redirection
	curl_easy_setopt(pDwldHandle->pCurl, CURLOPT_CONNECTTIMEOUT, DEFAULT_CONN_TIME_OUT);
	curl_easy_setopt(pDwldHandle->pCurl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
	curl_easy_setopt(pDwldHandle->pCurl, CURLOPT_LOW_SPEED_LIMIT, DOWNLOAD_MIN_SPEED);
	curl_easy_setopt(pDwldHandle->pCurl, CURLOPT_LOW_SPEED_TIME, DEFAULT_READ_TIME_OUT);

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	iRet = pthread_create(&pDwldHandle->threadDwldr, &attr, downloadThread, pDwldHandle);
	pthread_attr_destroy(&attr);

	if(0 != iRet) {
		logPrint(logHandle, LogLevelError, "Error creating download thread");
		return NULL;
	}
	logPrint(logHandle, LogLevelInfo, "Download thread created");

	logPrint(logHandle, LogLevelInfo, "Exiting Downloader initialization");
	return (SGN_DwldHandle)pDwldHandle;
}

int startDownload(SGN_DwldHandle pHandle, char *pSzUrl, void *pUserData, char *pSzPost,
						sgn_bool isHead, sgn_bool isFile, SGNCB_DwldComplete pDwldComplete, int iReqId) {
	if(NULL == pHandle || NULL == pSzUrl || NULL == pDwldComplete) {
		return -1;
	}
	SGN_Downloader *pDwldHandle = (SGN_Downloader *)pHandle;
	logPrint(pDwldHandle->pLogHandle, LogLevelInfo, "Entering startDownload");
	SGN_DwldReq *pDwlReq = calloc(sizeof(SGN_DwldReq), 1);
	if(NULL == pDwlReq) {
		logPrint(pDwldHandle->pLogHandle, LogLevelError, "Error allocating download packet");
		return -1;
	}
	pDwlReq->iReqId		= iReqId;
	pDwlReq->isFile		= isFile;
	pDwlReq->isHead		= isHead;
	pDwlReq->isQuit		= sgn_false;
	pDwlReq->pDwldHandle= (SGN_Downloader *)pHandle;
	pDwlReq->pUrl		= sgn_strdup(pSzUrl);
	pDwlReq->pPayload	= sgn_strdup(pSzPost);
	pDwlReq->pUserData	= pUserData;
	pDwlReq->pDwldComplete	= pDwldComplete;

	if(pDwlReq->isHead == sgn_false) {
		if(pDwlReq->isFile) {
			pDwlReq->pAssetFile = fopen(DEFAULT_DOWNLOAD_FILE_NAME, "wb");
			if(NULL == pDwlReq->pAssetFile) {
				logPrint(pDwldHandle->pLogHandle, LogLevelError, "Error creating file for download");
				return -1;
			}
		} else {
			pDwlReq->pRespString = sgn_string_sized_new(0);
		}
	}
	sgn_async_queue_push(pDwldHandle->pDwlReqQ, pDwlReq);
	logPrint(pDwldHandle->pLogHandle, LogLevelInfo, "Exiting startDownload");
	return 0;
}

void *downloadThread(void *pUserInfo) {
	if(NULL == pUserInfo) {
		return NULL;
	}

	SGN_Downloader *pDwldHandle = pUserInfo;
	logPrint(pDwldHandle->pLogHandle, LogLevelInfo, "Download thread starting...");
	while(1) {
		SGN_DwldReq *pDwlReq	= sgn_async_queue_pop(pDwldHandle->pDwlReqQ);
		if(NULL == pDwlReq) {
			logPrint(pDwldHandle->pLogHandle, LogLevelError, "Error popping download request");
			continue;
		}
		if(pDwlReq->isQuit) {
			logPrint(pDwldHandle->pLogHandle, LogLevelInfo, "Quitting download thread");
			return NULL;
		}

		//	clear a little structures
		curl_easy_setopt(pDwldHandle->pCurl, CURLOPT_POSTFIELDS, NULL);
		curl_easy_setopt(pDwldHandle->pCurl, CURLOPT_HEADERFUNCTION, NULL);
		curl_easy_setopt(pDwldHandle->pCurl, CURLOPT_WRITEHEADER, NULL);

		curl_easy_setopt(pDwldHandle->pCurl, CURLOPT_URL, pDwlReq->pUrl);
		curl_easy_setopt(pDwldHandle->pCurl, CURLOPT_WRITEDATA, pDwlReq);

		if(pDwlReq->isHead) {
			curl_easy_setopt(pDwldHandle->pCurl, CURLOPT_NOBODY, 1);
			curl_easy_setopt(pDwldHandle->pCurl, CURLOPT_HEADERFUNCTION, headerCallback);
			curl_easy_setopt(pDwldHandle->pCurl, CURLOPT_WRITEHEADER, pDwlReq);
		} else {
			if(pDwlReq->isFile) {
				curl_easy_setopt(pDwldHandle->pCurl, CURLOPT_WRITEFUNCTION, writeToFile);
			} else {
				curl_easy_setopt(pDwldHandle->pCurl, CURLOPT_WRITEFUNCTION, writeToBuffer);
			}
			curl_easy_setopt(pDwldHandle->pCurl, CURLOPT_POSTFIELDS, pDwlReq->pPayload);
			curl_easy_setopt(pDwldHandle->pCurl, CURLOPT_POSTFIELDSIZE, (long)sgn_strlen(pDwlReq->pPayload));
		}
		int iRet = sendRequest((SGN_DwldHandle)pDwldHandle);
		if(DOWNLOAD_NO_ERROR == iRet && pDwlReq->pAssetFile) {
			char *pAssetName	= sgn_get_name_from_url(pDwlReq->pUrl);
			fclose(pDwlReq->pAssetFile); pDwlReq->pAssetFile = NULL;
			rename(DEFAULT_DOWNLOAD_FILE_NAME, pAssetName);
		}

		char *pBuffer	= NULL;
		if(pDwlReq->isHead) {
			pBuffer		= calloc(sizeof(unsigned int), 1);
			memcpy((void *)pBuffer, &pDwlReq->uiSizeFromHead, sizeof(unsigned int));
		} else {
			pBuffer = (pDwlReq->pRespString) ? pDwlReq->pRespString->str : NULL;
		}
		pDwlReq->pDwldComplete(iRet, pBuffer, pDwlReq->pUserData, pDwlReq->iReqId);

		sgn_string_free(pDwlReq->pRespString, sgn_false);
		freeDownloadReq(pDwlReq);
	}
	return NULL;
}

size_t headerCallback(void *pData, size_t size, size_t nmemb, void *userdata) {
	size_t realsize = size * nmemb;
	SGN_DwldReq *pDwldReq = (SGN_DwldReq *)userdata;

	char *pHeader			= pData;
	pHeader[realsize - 1]	= '\0';

	char *lenField = strstr(pHeader, "Content-Length:");
	if (lenField != NULL) {
		pDwldReq->uiSizeFromHead = strtol(lenField + 15, NULL, 10);
	}
	return realsize;
}

size_t writeToFile(void *pData, size_t size, size_t nmemb, void *pUserData) {
	size_t realsize = size * nmemb;
	SGN_DwldReq *pDwldReq = (SGN_DwldReq *)pUserData;

	size_t ret = fwrite(pData, 1, realsize, pDwldReq->pAssetFile);
	return ret;
}

size_t writeToBuffer(void *pData, size_t size, size_t nmemb, void *pUserData) {
	size_t realsize = size * nmemb;
	SGN_DwldReq *pDwlReq = (SGN_DwldReq *)pUserData;

	if(0 < realsize && NULL != pDwlReq) {
		char *pBuf = calloc(realsize+1, 1);
		memcpy(pBuf, pData, realsize);
		sgn_string_append(pDwlReq->pRespString, pBuf);
		free(pBuf);
	}
	return realsize;
}

int sendRequest(SGN_DwldHandle pData) {
	int resendCount = 0;
	CURLcode curlResp = DOWNLOAD_CURL_ERROR;

	SGN_Downloader *pDwldHandle = (SGN_Downloader *)pData;
	while(NULL != pDwldHandle) {
		long respCode;
		curlResp = curl_easy_perform(pDwldHandle->pCurl);
		CURLcode infoResp = curl_easy_getinfo(pDwldHandle->pCurl, CURLINFO_RESPONSE_CODE, &respCode);
		resendCount++;

		logPrint(pDwldHandle->pLogHandle, LogLevelInfo, "CURL Resp: %s", curl_easy_strerror(curlResp));

		if(curlResp == CURLE_OK) {
			if(infoResp == CURLE_OK) {
				if(respCode == 401) {
					if(resendCount > 1) {
						logPrint(pDwldHandle->pLogHandle, LogLevelError, "re-send for 401 failed after %d attempts", resendCount);
						return DOWNLOAD_CURL_ERROR;
					}
					logPrint(pDwldHandle->pLogHandle, LogLevelInfo, "401 Received, re-sending with Auth");
					continue;	/*  re-send the request */
				}  else {
					logPrint(pDwldHandle->pLogHandle, LogLevelInfo, "CURL %d received", respCode);
					return ((respCode >= 200) && (respCode <= 299)) ? DOWNLOAD_NO_ERROR : DOWNLOAD_CURL_ERROR;
				}
			} else {
				logPrint(pDwldHandle->pLogHandle, LogLevelError, "CURL Error: %s", curl_easy_strerror(infoResp));
				return (CURLE_UNSUPPORTED_PROTOCOL == infoResp) ? DOWNLOAD_UNSUPPORTED_PROTOCOL : DOWNLOAD_CURL_ERROR;
			}
		} else {
			break;
		}
	}

	return (CURLE_UNSUPPORTED_PROTOCOL == curlResp)? DOWNLOAD_UNSUPPORTED_PROTOCOL : DOWNLOAD_CURL_ERROR;
}
