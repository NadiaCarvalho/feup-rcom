/*Non-Canonical Input Processing*/

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define FLAG 0x7E
#define SEND 0x03
#define RECEIVE 0x01
#define SET 0x03
#define UA 0x07

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
        int fd,c, res;
        struct termios oldtio,newtio;
        char buf[255];
        int i, sum = 0, speed = 0;

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

        printf("Insert your sentence here: ");
        gets(buf);

        /*testing*/
        //buf[25] = '\n';

        int written_chars = 0;

        while(written_chars < strlen(buf)+1) {
                res = write(fd,buf, strlen(buf)+1);
                if(res == 0) break;
                written_chars += res;
                printf("%d bytes written\n", res);

        }


        /*
           O ciclo FOR e as instru��es seguintes devem ser alterados de modo a respeitar
           o indicado no gui�o
         */

        char msg[256] = "";

        while (STOP==FALSE) { /* loop for input */
                res = read(fd,buf,255); /* returns after x chars have been input */
                buf[res]=0;     /* so we can printf... */
                strcat(msg, buf);
                if (buf[res-1]== 0) STOP=TRUE;
        }

        i = 0;
        while(msg[i] != '\0') {
                printf("%X ", msg[i]);
                fflush(stdout);
                i++;
        }

        if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
                perror("tcsetattr");
                exit(-1);
        }




        close(fd);
        return 0;
}


int parseFlag(char *packet) {

}

// F|A|C|BCC|F
int send_SET_flag(int fd) {
        char buf[5];

        buf[0] = FLAG;
        buf[1] = SEND;
        buf[2] = SET;
        buf[3] = buf[1] ^ buf[2];
        buf[4] = FLAG;

        int buf_len = 5;
        int res = 0;
        int written_chars = 0;

        while(written_chars < buf_len) {
                res = write(fd,buf, buf_len);
                if(res == 0) break;
                written_chars += res;
                printf("%d bytes written\n", res);
        }
}
