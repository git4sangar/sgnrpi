//sgn
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
 
int main(void) {
	printf("if directory exists\n");
	if(0 != access("/home/sangar/proj/rpi/ssr/", F_OK)) {
	if (ENOENT == errno) {
		 printf("directory does not exist\n");
		 if( 0 == mkdir("/home/sangar/proj/rpi/abcd/", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) ) {
			 printf("directory created\n");
		 }		 
	  }
	  if (ENOTDIR == errno) {
		 printf("it is not a dir\n");
	  }
	}
	return 0;
}
