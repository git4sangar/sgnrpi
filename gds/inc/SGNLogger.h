//sgn
#ifndef __SGN_LOGGER_H_
#define __SGN_LOGGER_H_

#define SGN_LOG_MAX_SIZE_MB		(32)

typedef struct SGN_Logger *SGN_LogHandle;

typedef enum {
	LogLevelDebug,
	LogLevelError,
	LogLevelFatal,
	LogLevelWarn,
	LogLevelInfo
}SGNLogLevel;

SGN_LogHandle initLogger(unsigned int maxFileSizeMB);
int logPrint(SGN_LogHandle logHandle, SGNLogLevel level, char * format, ...);

#endif
