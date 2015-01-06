//gn

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "SGNLogger.h"

typedef struct {
	char *pDeviceId;
	char *pSWVer;
	int iMaxLogFileSizeInMB;
	FILE *pLogFile;
} SGN_Logger;

SGN_LogHandle initLogger(unsigned int maxFileSizeMB) {
	SGN_Logger *pLogHandle = malloc(sizeof(SGN_LogHandle));

	if(SGN_LOG_MAX_SIZE_MB < maxFileSizeMB) {
		printf("Fatal error. No memory for allocating log handle\n");
	}

	if(NULL == pLogHandle) {
		printf("Fatal error. No memory for allocating log handle\n");
		return NULL;
	}

	return (SGN_LogHandle)pLogHandle;
}


int logPrint(SGN_LogHandle logHandle, SGNLogLevel level, char * pFormat, ...) {
	va_list argList;
	va_start(argList, pFormat);
	vprintf(pFormat, argList);
	printf("\n");
	va_end(argList);
	return 0;
}
