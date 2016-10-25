#include "data_link_layer.h"

int set_up_connection(char *terminal, struct termios *oldtio, status stat) {
  if (stat != TRANSMITTER && stat != RECEIVER) {
    printf("application_layer :: set_up_connection() :: Invalid status.\n");
    return -1;
  }

  application.app_layer_status = stat;

  int port;
  if (strcmp(terminal, COM1_PORT) == 0)
    port = COM1;
  else if (strcmp(terminal, COM2_PORT) == 0)
    port = COM2;
  else {
    printf("application_layer :: set_up_connection() :: Invalid terminal\n");
  }

  if ((application.file_descriptor =
           ll_open(port, application.app_layer_status)) < 0) {
    printf("application_layer :: set_up_connection() :: ll_open failed\n");
    return -1;
  }

  /*struct termios newtio;
  if (set_terminal_attributes(oldtio, &newtio) != 0) {
    printf("application_layer :: set_up_connection() :: error setting terminal "
           "attributes\n");
    return -1;
  }*/

  return application.file_descriptor;
}
