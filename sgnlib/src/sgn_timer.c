/*
 * sgn_timer.c
 *
 *  Created on: 30-Dec-2014
 *      Author: sangar
 */

#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <strings.h>
#include "sgn_list.h"
#include "sgn_timer.h"

typedef struct {
	void *pUserData;
	int secs;
}SGN_Ad;

typedef struct {
	timer_t timerId;
	sgn_list_t *pAds;
	pthread_mutex_t timerMutex;
	SGNCB_TimerExpire pTimerCallback;
}SGN_Timer;

SGN_TimerHandle sgn_create_timer(SGNCB_TimerExpire pTimerCB) {
	if(NULL == pTimerCB) {
		return NULL;
	}
	SGN_Timer *pSgnTimer = calloc(sizeof(SGN_Timer), 1);
	if(NULL != pSgnTimer) {
		pSgnTimer->pTimerCallback = pTimerCB;
	}

	pthread_mutex_init(&pSgnTimer->timerMutex, NULL);

	timer_t timer_id;
	struct sigevent sigev;
	bzero(&sigev, sizeof(struct sigevent));

	sigev.sigev_notify = SIGEV_THREAD;
	sigev.sigev_notify_function = (void (*)(sigval_t))getTimerKicks;
	sigev.sigev_value.sival_ptr = pSgnTimer;

	int iRet = timer_create(CLOCK_MONOTONIC, &sigev, &timer_id);
	if(0 != iRet) {
		return NULL;
	}
	pSgnTimer->timerId = timer_id;
	return (SGN_TimerHandle)pSgnTimer;
}

int sgn_start_timer(SGN_TimerHandle hTimer) {
	SGN_Timer *pSgnTimer = (SGN_Timer *)hTimer;
	if(NULL == pSgnTimer) {
		return -1;
	}
	struct itimerspec timespec;
	timespec.it_value.tv_sec = 1;
	timespec.it_value.tv_nsec = 0;
	timespec.it_interval.tv_sec = 1;
	timespec.it_interval.tv_nsec = 0;

	int iRet = timer_settime(pSgnTimer->timerId, 0, &timespec, NULL);
	return iRet;
}

int sgn_addto_timer(SGN_TimerHandle hTimer, int secs, void *pUserData) {
	SGN_Timer *pSgnTimer	= (SGN_Timer *)hTimer;
	if(NULL == pSgnTimer) {
		return -1;
	}
	SGN_Ad *pAd		= calloc(sizeof(SGN_Ad), 1);
	pAd->pUserData	= pUserData;
	pAd->secs		= secs;
	pthread_mutex_lock(&pSgnTimer->timerMutex);
	pSgnTimer->pAds = sgn_list_append(pSgnTimer->pAds, pAd);
	pthread_mutex_unlock(&pSgnTimer->timerMutex);

	return 0;
}

void *getTimerKicks(void *pData) {
	sgn_list_t *pAds = NULL, *pNext = NULL;
	sgn_list_t *pCBs = NULL;
	SGN_Ad *pAd = NULL;

	SGN_Timer *pSgnTimer	= pData;
	if(NULL == pSgnTimer) {
		return NULL;
	}

	pthread_mutex_lock(&pSgnTimer->timerMutex);
	pSgnTimer->pAds = sgn_list_first( pSgnTimer->pAds );
	for(pAds = pSgnTimer->pAds; pAds != NULL;) {
		pNext	= pAds->next;
		pAd		= pAds->data;
		pAd->secs--;
		if(0 >= pAd->secs) {
			//	Invoke all the callbacks after unlocking the mutex
			pCBs = sgn_list_append(pCBs, pAd->pUserData);
			pSgnTimer->pAds = sgn_list_remove(pSgnTimer->pAds, pAd);
			free(pAd); pAd = NULL;
		}
		pAds = pNext;
	}
	pthread_mutex_unlock(&pSgnTimer->timerMutex);

	sgn_list_t *pCB = sgn_list_first(pCBs);
	while(pCB) {
		pSgnTimer->pTimerCallback((SGN_TimerHandle)pSgnTimer, pCB->data);
		pCB = pCB->next;
	}
	sgn_list_free( sgn_list_first(pCBs) );
	return NULL;
}

int sgn_stop_timer(SGN_TimerHandle hTimer) {
	SGN_Timer *pSgnTimer	= (SGN_Timer *)hTimer;
	struct itimerspec timespec;
	if(NULL == pSgnTimer) {
		return -1;
	}

	bzero(&timespec, sizeof(struct itimerspec));
	int iRet = timer_settime(pSgnTimer->timerId, 0, &timespec, NULL);
	return iRet;
}

int sgn_destroy_timer(SGN_TimerHandle hTimer) {
	SGN_Timer *pSgnTimer	= (SGN_Timer *)hTimer;

	if(NULL == pSgnTimer) {
		return -1;
	}
	sgn_stop_timer(hTimer);

	pthread_mutex_lock(&pSgnTimer->timerMutex);
	sgn_list_t *pAds = sgn_list_first(pSgnTimer->pAds);
	while(pAds) {
		free(pAds->data);
		pAds = pAds->next;
	}
	sgn_list_free( sgn_list_first(pSgnTimer->pAds) );
	pthread_mutex_unlock(&pSgnTimer->timerMutex);

	free(pSgnTimer); pSgnTimer = NULL;

	return 0;
}
