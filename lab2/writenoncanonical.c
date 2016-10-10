/*Non-Canonical Input Processing*/

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

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

int is_UA(char *msg) {
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

        printf("Sending SET packet...\n");
        send_US_frame(fd, SET);
        printf("SET packet sent.\n");
        /*
           O ciclo FOR e as instru��es seguintes devem ser alterados de modo a respeitar
           o indicado no gui�o
         */

        char msg[255] = "";
       	int msg_len;
	
	while(!is_UA(msg)) {
	     read_message(fd, msg, &msg_len);
        print_as_hexadecimal(msg, msg_len);
}

	printf("\nReceived UA.\n");

        if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
                perror("tcsetattr");
                exit(-1);
        }

        close(fd);
        return 0;
}
