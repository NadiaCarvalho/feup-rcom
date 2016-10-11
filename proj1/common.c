#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "common.h"

volatile int STOP=FALSE;

int send_message(int fd, char* buf, int buf_length) {
        int written_chars = 0;
        int res = 0;

        while(written_chars < buf_length) {
                res = write(fd,buf, buf_length);
                if(res == 0) break;
                written_chars += res;
        }

        return 0;
}


int read_message(int fd, char* msg, int* msg_len) {
	int res = 0;
        char buf[255];
        *msg_len = 0;

        while (STOP==FALSE) {   /* loop for input */
                res = read(fd,buf,255); /* returns after x chars have been input */

		if(res > 0) {
        	       if (buf[res-1]== 0)  STOP=TRUE;
	                memcpy(msg+(*msg_len), buf, res);
                	(*msg_len) += res;
		} else {
			if(errno == EINTR)		
				return -1;
			else
				return 0;
		}
        }

        return 0;
}

void print_as_hexadecimal(char *msg, int msg_len) {
        int i;
        for(i = 0; i < msg_len; i++)
                printf("%02X \n", msg[i]);
        fflush(stdout);
}

int send_US_frame(int fd, int control_bit) {
        char buf[6];

        buf[0] = FLAG;
        buf[1] = SEND;
        buf[2] = control_bit;
        buf[3] = buf[1] ^ buf[2];
        buf[4] = FLAG;
	buf[5] = 0;

        int buf_len = 6;

	return send_message(fd, buf, buf_len);
}
