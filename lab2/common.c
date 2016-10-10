#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "common.h"

volatile int STOP=FALSE;

int send_message(int fd, char* buf, int buf_length) {
        int written_chars = 0;
        int res = 0;

        while(written_chars < buf_length) {
                res = write(fd,buf, buf_length);
                if(res == 0) break;
                written_chars += res;
                printf("%d bytes written.\n", res);
        }

        return 0;
}


int read_message(int fd, char* msg, int* msg_len) {
	int res = 0;
        char buf[255];
        *msg_len = 0;

        while (STOP==FALSE) {   /* loop for input */
                res = read(fd,buf,255); /* returns after x chars have been input */

                if (buf[res-1]== 0)  STOP=TRUE;
                memcpy(msg+(*msg_len), buf, res);
                (*msg_len) += res;
        }

        return 0;
}

void print_as_hexadecimal(char *msg, int msg_len) {
        int i;
        for(i = 0; i < msg_len; i++)
                printf("%02X ", msg[i]);
        fflush(stdout);
}
