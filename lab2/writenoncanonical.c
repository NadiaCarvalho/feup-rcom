/*Non-Canonical Input Processing*/

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "common.h"

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

int retries = 0;

void retry(int signum) {
	retries++;	
	if(retries < 3)
		printf("Connection timed out. Retrying %d out of %d...\n", retries, RETRY);
	else
		printf("Connection timed out %d times. Packet not received.\n", retries);
}

int is_UA(char *msg, int msg_len) {
	if(msg_len >= 5)
		return 1;
	
	return (msg[0] == FLAG && msg[1] == SEND && msg[2] == UA && msg[3] == (msg[1] ^ msg[2]) && msg[4] == FLAG);
}

int main(int argc, char** argv)
{
        int fd;
        struct termios oldtio,newtio;

        if ( (argc < 2) ||
             ((strcmp("/dev/ttyS0", argv[1])!=0) &&
              (strcmp("/dev/ttyS1", argv[1])!=0) )) {
                printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
                exit(1);
        }


        /*
           Open serial port device for reading and writing and not as controlling tty
           because we don't want to get killed if linenoise sends CTRL-C.
         */


        fd = open(argv[1], O_RDWR | O_NOCTTY );
        if (fd <0) {perror(argv[1]); exit(-1); }

        if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
                perror("tcgetattr");
                exit(-1);
        }

        bzero(&newtio, sizeof(newtio));
        newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
        newtio.c_iflag = IGNPAR;
        newtio.c_oflag = 0;

        /* set input mode (non-canonical, no echo,...) */
        newtio.c_lflag = 0;

        newtio.c_cc[VTIME]    = 0;/* inter-character timer unused */
        newtio.c_cc[VMIN]     = 1;/* blocking read until 5 chars received */



        /*
           VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
           leitura do(s) pr�ximo(s) caracter(es)
         */

        tcflush(fd, TCIOFLUSH);

        if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
                perror("tcsetattr");
                exit(-1);
        }

        printf("New termios structure set\n");

        //REad the message from stdin and send it.

        /*printf("Insert your sentence here: ");

           gets(buf);
           send_message(fd, buf, strlen(buf)+1);*/

	struct sigaction new_action, old_action;
	
	new_action.sa_handler = retry;
	new_action.sa_flags &= !SA_RESTART; //Needed in order to block read from restarting.

	sigaction(SIGALRM, &new_action, &old_action);
	printf("Setting alarm handler...\n");
	
        char msg[255] = "";
       	int msg_len;
	
	while(retries < 3) {
	        printf("Sending SET packet...\n");
        	send_US_frame(fd, SET);
	        printf("SET packet sent.\n");
		alarm(3);

       		if(!read_message(fd, msg, &msg_len)) {
			if(is_UA(msg, msg_len))
					break;
		}	
	}

	if(retries == RETRY) {
		printf("UA not received.\n");
	} else {		
		printf("Received UA.\n");
		print_as_hexadecimal(msg, msg_len);
	}	

	retries = 0;
        

        if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
                perror("tcsetattr");
                exit(-1);
        }

        close(fd);
        return 0;
}
