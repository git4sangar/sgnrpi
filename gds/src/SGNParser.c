/* SGN
 * SGNParser.c
 *
 *  Created on: 01-Jan-2015
 *      Author: sangar
 */

#include <stdio.h>
#include <json/json.h>
#include <stdlib.h>
#include <time.h>
#include <sgn_utils.h>
#include "SGNParser.h"
#include "SGNLogger.h"
#include "sgn_list.h"
#include "sgn_string.h"

sgn_list_t *parsePlaylist(char *pJsonPacket, SGN_LogHandle hLogHandle) {
	int iLoop = 0;
	json_object *pAdObj = NULL;
	json_object *pJobj	= NULL;
	sgn_list_t *pPlaylist = NULL;

	if(NULL == pJsonPacket || NULL == hLogHandle) {
		return NULL;
	}
	logPrint(hLogHandle, LogLevelInfo, "Entering parsePlaylist");

	json_object *pJRoot		= json_tokener_parse(pJsonPacket);
	json_object *pJVer		= json_object_object_get(pJRoot, "ver");
	json_type eType			= json_object_get_type(pJVer);
	if(json_type_string != eType ||
			sgn_strcmp(SGN_PLAYLIST_VERSION, json_object_get_string(pJVer)) ) {
		logPrint(hLogHandle, LogLevelError, "Error parsing playlist version");
		return NULL;
	}
	json_object	*pJAds		= json_object_object_get(pJRoot, "ads");

	int iNoOfAds			= json_object_array_length(pJAds);
	logPrint(hLogHandle, LogLevelInfo, "There are %d ads in the playlist", iNoOfAds);

	for(iLoop = 0; iLoop < iNoOfAds; iLoop++) {
		PlaylistData *pAd	= calloc(sizeof(PlaylistData), 1);
		pAdObj	= json_object_array_get_idx(pJAds, iLoop);
		pJobj	= NULL;

		pJobj			= json_object_object_get(pAdObj, "id");
		pAd->iAdId		= json_object_get_int(pJobj);

		pJobj			= json_object_object_get(pAdObj, "url");
		pAd->pUrl		= sgn_strdup(json_object_get_string(pJobj));
		if(NULL == sgn_get_name_from_url(pAd->pUrl)) {
			logPrint(hLogHandle, LogLevelFatal, "Download url: %s without file name", pAd->pUrl);
			reutrn NULL;
		}

		pJobj			= json_object_object_get(pAdObj, "type");
		pAd->eAdType	= json_object_get_int(pJobj);

		pJobj			= json_object_object_get(pAdObj, "format");
		pAd->eFormat	= json_object_get_int(pJobj);

		pJobj			= json_object_object_get(pAdObj, "width");
		pAd->iWidth		= json_object_get_int(pJobj);

		pJobj			= json_object_object_get(pAdObj, "height");
		pAd->iHeight	= json_object_get_int(pJobj);

		pJobj			= json_object_object_get(pAdObj, "aratio");
		pAd->eAspectRatio= json_object_get_int(pJobj);

		pJobj			= json_object_object_get(pAdObj, "trkrS");
		pAd->pTrkrS		= sgn_strdup(json_object_get_string(pJobj));

		pJobj			= json_object_object_get(pAdObj, "trkrE");
		pAd->pTrkrE		= sgn_strdup(json_object_get_string(pJobj));

		pJobj			= json_object_object_get(pAdObj, "nextAd");
		pAd->iNextAd	= json_object_get_int(pJobj);

		pJobj			= json_object_object_get(pAdObj, "prevAd");
		pAd->iPrevAd	= json_object_get_int(pJobj);

		pJobj			= json_object_object_get(pAdObj, "date_time");
		if(pJobj) {
			char *pDateTime	= sgn_strdup(json_object_get_string(pJobj));
			pAd->pDateTime	= calloc(sizeof(struct tm), 1);
			strptime(pDateTime, "%Y-%m-%d %H:%M:%S", pAd->pDateTime);
			free(pDateTime);
		}

		logPrint(hLogHandle, LogLevelInfo, "Appending AdId: %d to list\n", pAd->iAdId);
		pPlaylist		= sgn_list_append(pPlaylist, pAd);
	}

	logPrint(hLogHandle, LogLevelInfo, "Exiting parsePlaylist");
	return pPlaylist;
}
