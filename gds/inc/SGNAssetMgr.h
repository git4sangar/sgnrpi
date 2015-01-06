/*
 * SGNAssetMgr.h
 *
 *  Created on: 02-Jan-2015
 *      Author: sangar
 */

#ifndef SGNASSETMGR_H_
#define SGNASSETMGR_H_

#include "SGNParser.h"
#include "SGNDownloader.h"
#include "SGNLogger.h"
#include "sgn_list.h"

#define ASSET_LIST_FILE		"assets_list.txt"
#define ASSET_DIR			"~/DigitalSignage/"

typedef struct SGN_AssetMgr *SGN_AssetMgrHandle;
typedef void(*SGNCB_AssetDwldComplete)(sgn_bool status, void *pUserData);

SGN_AssetMgrHandle initAssetManager(SGN_LogHandle hLogger, SGN_DwldHandle hDownloader,
										SGNCB_AssetDwldComplete pAssetDwldComplete, void *pUserData);
int downloadPlaylistData(SGN_AssetMgrHandle hAssetMgr, sgn_list_t *pAssetList);
int initiateHeadRequests(SGN_AssetMgrHandle hAssetMgr, sgn_list_t *pAssetList);
void headReqComplete(int status, int iReqId, sgn_bool isHead, void *pData);
void postReqComplete(int status, int iReqId, sgn_bool isHead, void *pData);
void *getDwldReqFromId(sgn_list_t *pDwldReqQ, int iReqId);
int loadAssetList(SGN_AssetMgrHandle hAssetMgr, char *pDirWithSlash, char *pFileName);
int makeHeadRequest(SGN_AssetMgr *pAssetMgr, sgn_list_t *pCurrent);
void *assetDownloadThread(void *pData);
#endif /* SGNASSETMGR_H_ */
