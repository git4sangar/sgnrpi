#include <stdio.h>
#include <unistd.h>
#include <string.h> /* for strncpy */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <curl/curl.h>


int main() {
 int fd;
 struct ifreq ifr;
 CURL *curl;
 
 CURL *curl_handle = NULL;
 curl_global_init(CURL_GLOBAL_ALL);
 curl = curl_easy_init();
 
 fd = socket(AF_INET, SOCK_DGRAM, 0);

 /* I want to get an IPv4 IP address */
 ifr.ifr_addr.sa_family = AF_INET;

 /* I want IP address attached to "eth0" */
 strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

 ioctl(fd, SIOCGIFADDR, &ifr);

 close(fd);

 /* display result */
 //printf("%s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

 char url[128];
 strcpy(url, "http://www.madhangi.com/set_rpi_ip.php?ip=no_ip_got_yet");
 sprintf(url, "http://www.madhangi.com/set_rpi_ip.php?ip=%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
 curl_easy_setopt(curl, CURLOPT_URL, url);
 curl_easy_perform(curl);
 curl_easy_cleanup(curl);
 
 return 0;
}
