/*Non-Canonical Input Processing*/

#include "data_link_layer.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

int is_UA(char *msg, int msg_len) {
  if (msg_len >= 5)
    return 1;

  return (msg[0] == FLAG && msg[1] == SEND && msg[2] == UA &&
          msg[3] == (msg[1] ^ msg[2]) && msg[4] == FLAG);
}

int main(int argc, char **argv) {
  int fd;
  struct termios oldtio;

  if ((argc < 2) || ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
                     (strcmp("/dev/ttyS1", argv[1]) != 0))) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }

  /*
     Open serial port device for reading and writing and not as controlling tty
     because we don't want to get killed if linenoise sends CTRL-C.
   */
  fd = ll_open(argv[1], &oldtio, TRANSMITTER);
  
  ll_close(fd, &oldtio);
  return 0;
}
