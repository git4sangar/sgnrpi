/*
 * sgn_utils.c
 *
 *  Created on: 04-Jan-2015
 *      Author: sangar
 */

#include <pthread.h>

int gUniqueRunningNo;
pthread_mutex_t *pUniqueMtx = NULL;

int sgn_get_unique_no(void) {
	int iRet = 0;
	if(NULL == pUniqueMtx) {
		pUniqueMtx = calloc(sizeof(pthread_mutex_t), 1);
		pthread_mutex_init(pUniqueMtx, NULL);
	}

	pthread_mutex_lock(pUniqueMtx);
	iRet = ++gUniqueRunningNo;
	pthread_mutex_unlock(pUniqueMtx);
	return iRet;
}

char *sgn_get_name_from_url(char *pURL) {
	char *ptr = NULL;
	char *prevPtr = NULL;

	if (!pURL)	return NULL;

	ptr = strstr(pURL, "://");
	if (!ptr)
		return NULL;

	ptr += 3;
	while ((ptr = strchr(ptr, '/')) != NULL) {
		ptr++;
		prevPtr = ptr;
	}

	if (prevPtr && *pprevPtr) {
		return sgn_strdup(prevPtr);
	}

	return NULL;
}
