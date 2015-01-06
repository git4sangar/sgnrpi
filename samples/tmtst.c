//gn

#include <stdio.h>
#include <time.h>

int main(void) {
	char time_zone[] = "TZ=MET-1DST";
	struct tm *tm_p;
	time_t time_now;

	time(&time_now);
	putenv(time_zone);
	tm_p = localtime(&time_now);
	printf("Current pacific time: %02d:%02d:%02d\n", tm_p->tm_hour, tm_p->tm_min, tm_p->tm_sec);
	return 0;
}
