/* sgn
 * sgn_timer.h
 *
 *  Created on: 30-Dec-2014
 *      Author: sangar
 */

#ifndef SGN_TIMER_H_
#define SGN_TIMER_H_

typedef struct SGN_Timer *SGN_TimerHandle;
typedef void(*SGNCB_TimerExpire)(SGN_TimerHandle pHandle, void *pUserData);

//	Creates a timer
SGN_TimerHandle sgn_create_timer(SGNCB_TimerExpire pTimerCB);

//	Inserts an item to timer
int sgn_addto_timer(SGN_TimerHandle hTimer, int secs, void *pUserData);

//	Starts the timer. Returns 0 only on success.
int sgn_start_timer(SGN_TimerHandle hTimer);

//	Stops the timer. Returns 0 only on success.
int sgn_stop_timer(SGN_TimerHandle hTimer);

//	Destroy the timer. Returns 0 only on success.
int sgn_destroy_timer(SGN_TimerHandle);

//	SigevNotify function
void *getTimerKicks(void *pData);
#endif /* SGN_TIMER_H_ */
