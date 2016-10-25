/*Non-Canonical Input Processing*/

#include "data_link_layer.h"
#include "application_layer.h"
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

int main(int argc, char **argv) {
  if ((argc < 2) || ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
                     (strcmp("/dev/ttyS1", argv[1]) != 0))) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }

  /*
     Open serial port device for reading and writing and not as controlling tty
     because we don't want to get killed if linenoise sends CTRL-C.
   */
  int fd = set_up_connection(argv[1], TRANSMITTER);

  if(fd < 0) {
    printf("Error opening file descriptor. Exiting...\n");
    return -1;
  }

  char msg[255];
  strcpy(msg, "Ola, sou o bernardo");
  char msg_len = strlen(msg);
  char start_packet[] = {2, 0, 1, msg_len};
  char end_packet[] = {3};
  char information_packet[255];
  information_packet[0] = 1;
  information_packet[1] = 0;
  information_packet[2] = 0;
  information_packet[3] = msg_len;
  memcpy(information_packet+4, msg, msg_len);

  ll_write(fd, start_packet, 4);
  ll_write(fd, information_packet, msg_len + 4);
  ll_write(fd, end_packet, 1);

  ll_close(fd);
  return 0;
}
