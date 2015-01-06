//sgn
#ifndef __SGN_DOWNLOADER_H_
#define __SGN_DOWNLOADER_H_

#include "SGNLogger.h"
#include "sgn_types.h"

#define DOWNLOAD_DATA_ALLOC_SIZE		(10240)	// size for downloading
#define DEFAULT_CONN_TIME_OUT			(5)		// timeout for connecting to server
#define DEFAULT_READ_TIME_OUT			(5)		// after connection download timeout
#define DOWNLOAD_MIN_SPEED				(3000)	// minimum speed below which download aborts
#define DEFAULT_DOWNLOAD_FILE_NAME		"tempFile.dat"

#define		DOWNLOAD_NO_ERROR				(0)
#define		DOWNLOAD_UNSUPPORTED_PROTOCOL	(1)
#define		DOWNLOAD_CURL_ERROR				(2)
#define		DOWNLOAD_ERROR					(3)
#define		DOWNLOAD_INVALID_INPUT			(4)

// pData -> to be freed
typedef void(*SGNCB_DwldComplete)(int status, void *pHeadOrBuf, void *pUserData, int iReqId);
typedef struct SGN_Downloader *SGN_DwldHandle;

SGN_DwldHandle initDownloader(SGN_LogHandle logHandle);
int startDownload(SGN_DwldHandle pHandle, char *pSzUrl, void *pUserData, char *pSzPost,
						sgn_bool isHead, sgn_bool isFile, SGNCB_DwldComplete pDwldComplete, int iReqId);
void *downloadThread(void *);
int deinitDownloader(SGN_DwldHandle pHandle);

size_t headerCallback(void *ptr, size_t size, size_t nmemb, void *userdata);
size_t writeToFile(void *ptr, size_t size, size_t nmemb, void *file);
size_t writeToBuffer(void *ptr, size_t size, size_t nmemb, void *data);
int sendRequest(SGN_DwldHandle pDwldHandle);

#endif
