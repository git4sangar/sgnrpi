//gn

#include <stdio.h>
#include <time.h>

int main(void) {
	struct tm * timeinfo;
	time_t rawtime, curTime, newTime;
	curTime = time(NULL);

	time( &rawtime );
	timeinfo = localtime( &rawtime );
	timeinfo->tm_mday = 31;
	timeinfo->tm_sec = 30;
	timeinfo->tm_min = 30;
	timeinfo->tm_hour = 10;
	newTime = mktime(timeinfo);

	printf("epoch: %d, new epoch: %d\n", curTime, newTime);
	return 0;
}
