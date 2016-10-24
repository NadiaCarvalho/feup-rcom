#include "data_link_layer.h"
#include "application_layer.h"

int ll_open(char *terminal, struct termios *old_port_settings, Status status) {
  if (status != TRANSMITTER && status != RECEIVER) {
    printf("Invalid status.\n");
    return -1;
  }

  data_link_layer.status = status;

  int fd = open(terminal, O_RDWR | O_NOCTTY);

  if (fd < 0) {
    printf("Error opening terminal '%s'\n", terminal);
    return -1;
  }

  if (tcgetattr(fd, old_port_settings) == -1) { /* save current port settings */
    printf("Error getting port settings.\n");
    close(fd);
    return -1;
  }

  struct termios new_port_settings;

  bzero(&new_port_settings, sizeof(new_port_settings));
  new_port_settings.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  new_port_settings.c_iflag = IGNPAR;
  new_port_settings.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  new_port_settings.c_lflag = 0;

  new_port_settings.c_cc[VTIME] =
      0; /* inter-character timer unused in 1/10th of a second*/
  new_port_settings.c_cc[VMIN] = 1; /* blocking read until x chars received */

  tcflush(fd, TCIOFLUSH);

  if (tcsetattr(fd, TCSANOW, &new_port_settings) == -1) {
    printf("Error setting port settings.\n");
    close(fd);
    return -1;
  }

  int frame_len;
  if (status == TRANSMITTER) {
    char *frame = create_US_frame(&frame_len, SET);
    if (send_frame(fd, frame, frame_len, is_frame_UA) == -1) {
      close(fd);
      return -1;
    }
  } else {
    char msg[255];
    int msg_len;
    read_from_tty(fd, msg, &msg_len);
    char *frame = create_US_frame(&frame_len, UA);
    write_to_tty(fd, frame, frame_len);
  }

  printf("Connection succesfully established.\n");
  return fd;
}

int is_frame_RR(char *reply) {
  return (
      reply[0] == FLAG &&
      reply[1] == ((data_link_layer.status == TRANSMITTER) ? SEND : RECEIVE) &&
      reply[2] == RR && reply[3] == (reply[1] ^ reply[2]) && reply[4] == FLAG);
}

char *create_I_frame(int *frame_len, char *packet, int packet_len) {
  char *frame =
      (char *)malloc((6 + packet_len) * sizeof(char)); // Lacks byte stuffing
  *frame_len = 6 + packet_len;

  frame[0] = FLAG;
  frame[1] = SEND;
  frame[2] = 0 | (1 << 6);
  frame[3] = 0xFF;
  memcpy(frame + 4, packet, packet_len);
  frame[packet_len + 4] = 0xFF;
  frame[packet_len + 5] = FLAG;

  return frame;
}

int write_to_tty(int fd, char *buf, int buf_length) {
  int total_written_chars = 0;
  int written_chars = 0;

  while (total_written_chars < buf_length) {
    written_chars = write(fd, buf, buf_length);
    if (written_chars == 0)
      break;
    total_written_chars += written_chars;
  }

  return 0;
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
