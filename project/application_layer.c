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

  struct termios newtio;
  if (set_terminal_attributes(oldtio, &newtio) != 0) {
    printf("application_layer :: set_up_connection() :: error setting terminal "
           "attributes\n");
    return -1;
  }

  return application.file_descriptor;
}

int set_terminal_attributes(struct termios *old_port_settings,
                            struct termios *new_port_settings) {

  if (tcgetattr(application.file_descriptor, old_port_settings) ==
      -1) { /* save current port settings */
    printf("Error getting port settings.\n");
    close(application.file_descriptor);
    return -1;
  }

  bzero(new_port_settings, sizeof(*new_port_settings));
  new_port_settings->c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  new_port_settings->c_iflag = IGNPAR;
  new_port_settings->c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  new_port_settings->c_lflag = 0;

  new_port_settings->c_cc[VTIME] =
      0; /* inter-character timer unused in 1/10th of a second*/
  new_port_settings->c_cc[VMIN] = 1; /* blocking read until x chars received */

  tcflush(application.file_descriptor, TCIOFLUSH);

  if (tcsetattr(application.file_descriptor, TCSANOW, new_port_settings) ==
      -1) {
    printf("Error setting port settings.\n");
    close(application.file_descriptor);
    return -1;
  }

  return 0;
}
