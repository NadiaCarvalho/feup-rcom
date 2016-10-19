#include "data_link_layer.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#define BAUDRATE B9600
#define ACCEPTABLE_TIMEOUTS 3

volatile int STOP = 0;
int connection_timeouts = 0;

void print_as_hexadecimal(char *msg, int msg_len) {
  int i;
  for (i = 0; i < msg_len; i++)
    printf("%02X ", msg[i]);
  fflush(stdout);
}

void timeout(int signum) { connection_timeouts++; }

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

int read_from_tty(int fd, char *frame, int *frame_len) {
  int read_chars = 0;
  char buf[255];
  *frame_len = 0;

  STOP = 0;
  while (!STOP) {                    /* loop for input */
    read_chars = read(fd, buf, 255); /* returns after x chars have been input */

    if (read_chars > 0) { // If characters were read
      if (buf[read_chars - 1] == 0)
        STOP = 1;

      memcpy(frame + (*frame_len), buf, read_chars);
      (*frame_len) += read_chars;
    } else {              // If no characters were read or there was an error
      if (errno == EINTR) // If the read() command was interrupted
        return -1;
      else
        return 0;
    }
  }

  return 0;
}

int is_frame_UA(char *reply, int reply_len) {
  return (reply[0] == FLAG && reply[1] == SEND && reply[2] == UA &&
          reply[3] == (reply[1] ^ reply[2]) && reply[4] == FLAG);
}

//is_reply_valid is a function that checks if the reply received is valid.
//If it is, the send_frame ends correctly. If not, it continues its loop.
int send_frame(int fd, char *frame, int len,
               int (*is_reply_valid)(char *, int)) {
  struct sigaction new_action, old_action;
  char reply[255];
  int reply_len;

  new_action.sa_handler = timeout;
  new_action.sa_flags &=
      !SA_RESTART; // Needed in order to block read from restarting.

  if (sigaction(SIGALRM, &new_action, &old_action) == -1)
    printf("Error installing new SIGALRM handler.\n");

  while (connection_timeouts < ACCEPTABLE_TIMEOUTS) {
    write_to_tty(fd, frame, len);
    alarm(3);

    if (read_from_tty(fd, reply, &reply_len) ==
        0) { // If the read() was successful
      if (is_reply_valid(reply, reply_len))
        break;
    }
  }

  if (sigaction(SIGALRM, &old_action, NULL) == -1)
    printf("Error setting SIGALRM handler to original.\n");

  if (connection_timeouts == ACCEPTABLE_TIMEOUTS)
    return -1;
  else
    return 0;
}

int send_US_frame(int fd, int control_bit) { // Debug function
  char buf[6];

  buf[0] = FLAG;
  buf[1] = SEND;
  buf[2] = control_bit;
  buf[3] = buf[1] ^ buf[2];
  buf[4] = FLAG;
  buf[5] = 0;

  int buf_len = 6;

  return write_to_tty(fd, buf, buf_len);
}

int ll_open(char *terminal, struct termios *old_port_settings, Status status) {
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

  if (status == TRANSMITTER) {
    char buf[6];
    buf[0] = FLAG;
    buf[1] = SEND;
    buf[2] = SET;
    buf[3] = buf[1] ^ buf[2];
    buf[4] = FLAG;
    buf[5] = 0;
    if (send_frame(fd, buf, 6, is_frame_UA) == -1) {
      close(fd);
      return -1;
    }
  } else {
    char msg[255];
    int msg_len;
    read_from_tty(fd, msg, &msg_len);
    char buf[6];
    buf[0] = FLAG;
    buf[1] = SEND;
    buf[2] = UA;
    buf[3] = buf[1] ^ buf[2];
    buf[4] = FLAG;
    buf[5] = 0;
    write_to_tty(fd, buf, 6);
  }

  printf("Connection established.\n");
  return fd;
}

int ll_write(int fd, char *msg, int len) {
  // Writes and checks for validity
  // Using send_frame

  return 0;
}

int ll_read(int fd, char *msg, int *len) {
  // Reads and checks for validity
  //

  return 0;
}

int ll_close(int fd, struct termios *old_port_settings) {
  // Send DISC to receiver
  // Wait for DISC
  // Send UA

  if (tcsetattr(fd, TCSANOW, old_port_settings) == -1)
    printf("Error settings old port settings.\n");

  if (close(fd)) {
    printf("Error closing terminal file descriptor.\n");
    return -1;
  }

  return 0;
}
