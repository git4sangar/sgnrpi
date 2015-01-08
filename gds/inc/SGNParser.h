/* SGN
 * SGNParser.h
 *
 *  Created on: 01-Jan-2015
 *      Author: sangar
 */

#ifndef SGNPARSER_H_
#define SGNPARSER_H_

#include <time.h>
#include "SGNLogger.h"
#include "sgn_types.h"
#include "sgn_list.h"

#define SGN_PLAYLIST_VERSION	"1.0"

typedef enum {
	eTypeImage = 1,
	eTypeVideo,
	eTypeAudio
}AssetType;

typedef enum {
	eJpg = 1,
	ePng,
	eMp4,
	eMkv
}AdFormat;

typedef enum {
	eRes4_3 = 1,
	eRes16_9,
	eRes21_9
}ARatio;

typedef struct {
	int iDay; int iMonth; int iYear;
}ADate;

typedef struct {
	int iHours; int iMins; int iSecs;
}ATime;

typedef struct {
	int			iAdId;
	char		*pUrl;
	char		*pTrkrS;
	char		*pTrkrE;
	AssetType	eAdType;
	AdFormat	eFormat;
	int			iWidth;
	int			iHeight;
	int			iNextAd;
	int			iPrevAd;
	ARatio		eAspectRatio;
	struct tm	*pDateTime;
	int			assetSize;
}PlaylistData;

sgn_list_t *parsePlaylist(char *pJsonPacket, SGN_LogHandle hLogHandle);

#endif /* SGNPARSER_H_ */
