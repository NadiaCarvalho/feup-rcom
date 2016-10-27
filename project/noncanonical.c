/*Non-Canonical Input Processing*/

#include "application_layer.h"
#include "data_link_layer.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

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
  int fd = set_up_connection(argv[1], RECEIVER);

  if (fd < 0) {
    printf("Error opening file descriptor. Exiting...\n");
    return -1;
  }

  receive_data();

  ll_close(fd);
  return 0;
}
