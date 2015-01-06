//sgn
#ifndef __SGN_SYSINFO_H_
#define __SGN_SYSINFO_H_

#define SGN_APP_VER		"SGN_1.0"
#define SGN_DEV_ID		"0xAB"

//	returns null terminated string containing the version of the software
char *getSWVersion(void);

//	returns null terminated string containing the device id
char *getDeviceId(void);

#endif
