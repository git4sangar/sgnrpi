#include <stdio.h>      /* puts, printf */
#include <time.h>       /* time_t, struct tm, time, localtime */
#include <stdlib.h>
#include <string.h>

int main ()
{
  time_t rawtime;
  struct tm * timeinfo;
  char pEnv[64] = "string" ;
  //setenv("TZ", "CET6CEST5 30,M4.5.0/02:00:00,M10.5.0/03:00:00", 1);
  setenv("TZ", "Australia/Hobart", 1);
  tzset();
  strcpy(pEnv, getenv("TZ"));
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  printf("Current local time and date: %s\n", asctime(timeinfo));
  printf("Env variable: %s\n", pEnv);
  return 0;
}
