#include <stdio.h>
#include <string.h>

int main(void) {

	int i,c;
    char mac[20];
	char realMac[] = "aa:bb:cc:11:22:33";
 
   strncpy(mac, realMac, 20);
    for(i=0, c=0;(i<20 && (mac[i] != '\0'));i++){
        if(mac[i] != ':'){
            mac[c++] = mac[i];
        }
    }
    mac[c] = '\0';
    printf("mac address: %s\n", mac);
	return 0;
}
