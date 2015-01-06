/* fpont 12/99 */
/* pont.net    */
/* udpClient.c */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h> /* memset() */
#include <sys/time.h> /* select() */ 

#define REMOTE_SERVER_PORT 4443
#define MAX_MSG 100

typedef struct {
	int iPackType;		// To notify VOD session start
	//string mocamac;		// mac address of the moca
	char array[1024];
	unsigned char session;		// 0 -> end of session, 1 -> start of session
}NotifyPacket;

int main(int argc, char *argv[]) {
  
  int sd, rc, i;
  struct sockaddr_in cliAddr, remoteServAddr;
  struct hostent *h;

  remoteServAddr.sin_family = AF_INET;
  remoteServAddr.sin_addr.s_addr = inet_addr("192.168.1.100");
  remoteServAddr.sin_port = htons(REMOTE_SERVER_PORT);

  /* socket creation */
  sd = socket(AF_INET,SOCK_DGRAM,0);


  /* send data */
  i = 5;
  NotifyPacket *pPkt = malloc(sizeof(NotifyPacket));

  for(i=0;i< 5;i++) {
	 pPkt->iPackType = i;pPkt->session = (i%2) ? '1' : '0';
	 
    rc = sendto(sd, pPkt, sizeof(NotifyPacket)+1, 0, (struct sockaddr *) &remoteServAddr, sizeof(remoteServAddr));
    printf("Packet %d sent\n", i);

    if(rc<0) {
      printf("%s: cannot send data %d \n",argv[0],i-1);
      close(sd);
      exit(1);
    }

  }
  
  return 1;

}
