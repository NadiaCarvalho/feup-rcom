#include "data_link_layer.h"
#include "application_layer.h"

app_layer application;

int ll_open(char *terminal, struct termios *old_port_settings, status app_layer_status) {
  if (status != TRANSMITTER && status != RECEIVER) {
    printf("Invalid status.\n");
    return -1;
  }

  application.status = app_layer_status;

  /**
   * Opening the serial port
   */
  int fd = open(terminal, O_RDWR | O_NOCTTY);
  applicaton.file_descriptor = fd;

  if (applicaton.file_descriptor < 0) {
    printf("Error opening terminal '%s'\n", terminal);
    return -1;
  }

  /**
   * Changing the terminal attributes
   */
  struct termios new_port_settings;
  if(set_terminal_attributes(old_port_settings, &new_port_settings) != 0){
    printf("Error setting terminal attributes\n");
    return -1;
  }

  if(set_up_connection(terminal, application.status) != 0){
    printf("Error setting up connection! Exiting.\n")
    return -1;
  }

  return fd;
}

int ll_write(int fd, char *packet, int packet_len) {
  // Writes and checks for validity
  // Using send_frame
  int frame_len;
  char *frame = create_I_frame(&frame_len, packet, packet_len);

  // send_frame()
  // write_to_tty(fd, frame, frame_len);
    if (send_frame(fd, frame, frame_len, is_frame_RR) == 0)
    printf("Received RR.\n");
  else
    printf("Didn't receive RR.\n");

  return 0;
}

int ll_read(int fd, char *msg, int *len) {
  // Reads and checks for validity
  //

  return 0;
}

int ll_close(int fd, struct termios *old_port_settings) {
  char *frame;
  int frame_len = 0;

  if (data_link_layer.status == TRANSMITTER) {
    frame = create_US_frame(&frame_len, DISC);
    send_frame(fd, frame, frame_len, is_frame_DISC);
    write_to_tty(fd, create_US_frame(&frame_len, UA), frame_len);
  } else {
    char msg[256];
    int msg_len = 0;
    read_from_tty(fd, msg, &msg_len);
    if (is_frame_DISC(msg)) {
      frame = create_US_frame(&frame_len, DISC);
      send_frame(fd, frame, frame_len, is_frame_UA);
    }
  }

  // Send DISC to receiver
  // Wait for DISC
  // Send UA

  if (tcsetattr(fd, TCSANOW, old_port_settings) == -1)
    printf("Error settings old port settings.\n");

  if (close(fd)) {
    printf("Error closing terminal file descriptor.\n");
    return -1;
  }

  printf("Connection succesfully closed.\n");
  return 0;
}

int set_terminal_attributes(struct termios *old_port_settings, struct termios *new_port_settings){

  if (tcgetattr(fd, old_port_settings) == -1) { /* save current port settings */
    printf("Error getting port settings.\n");
    close(fd);
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

  tcflush(fd, TCIOFLUSH);

  if (tcsetattr(fd, TCSANOW, new_port_settings) == -1) {
    printf("Error setting port settings.\n");
    close(fd);
    return -1;
  }

  return 0;
}
