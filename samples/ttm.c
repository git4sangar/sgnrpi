#include <stdio.h>
#include <time.h>

int main(void) {
	time_t raw;
	struct tm *pLocal;
	
	raw = time(0);
	setenv("TZ", "Asia/Kolkata", 1);
	tzset();
	printf("SGN: %d, TimeZone: %s\n", raw, getenv("TZ"));
	pLocal = localtime(&raw);
	printf ("Current local time and date: %s\n", asctime(pLocal), pLocal->tm_isdst);

	printf("epoch for this time: %d\n", mktime(pLocal));
	pLocal = localtime(&raw);
	printf("Current local time and date after an hour: %s\n", asctime(pLocal));

	return 0;
}
