//gn

#ifndef _DIGITAL_SIGNAGE_H_
#define _DIGITAL_SIGNAGE_H_

#include "SGNDownloader.h"
#include "SGNAssetMgr.h"

#define MAX_LOG_FILE_SIZE_MB (16)

typedef struct {
	SGN_LogHandle hLogHandle;
	SGN_DwldHandle hDownloader;
	SGN_AssetMgrHandle hAssetMgr;
}SGN_Handle;

#endif
