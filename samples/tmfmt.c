#include <time.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    struct tm tm;
    char buf[255];

	memset(&tm, 0, sizeof(struct tm));
    strptime("2001-11-12 18:31:01", "%Y-%m-%d %H:%M:%S", &tm);
    strftime(buf, sizeof(buf), "%d %b %Y %H:%M", &tm); // buf will be "12 Nov 2001 18:31"
    puts(buf);
    return 0;
}
