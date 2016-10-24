#include "data_link_layer.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#define BAUDRATE B9600
#define ACCEPTABLE_TIMEOUTS 3

volatile int STOP = 0;
int connection_timeouts = 0;

struct {
  Status status;
} data_link_layer;

void print_as_hexadecimal(char *msg, int msg_len) {
  int i;
  for (i = 0; i < msg_len; i++)
    printf("%02X ", msg[i]);
  fflush(stdout);
}

void timeout(int signum) { connection_timeouts++; }

char *create_US_frame(int *frame_len, int control_bit) {
  char *buf = (char *)malloc(6 * sizeof(char));
  buf[0] = FLAG;

  if (data_link_layer.status == TRANSMITTER) {
    if (control_bit == SET || control_bit == DISC)
      buf[1] = SEND;
    else
      buf[1] = RECEIVE;
  } else {
    if (control_bit == RR || control_bit == REJ || control_bit == UA)
      buf[1] = SEND;
    else
      buf[1] = RECEIVE;
  }

  buf[2] = control_bit;
  buf[3] = buf[1] ^ buf[2];
  buf[4] = FLAG;
  buf[5] = 0;
  *frame_len = 6;

  return buf;
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

int is_frame_UA(char *reply) {
  return (
      reply[0] == FLAG &&
      reply[1] == ((data_link_layer.status == TRANSMITTER) ? SEND : RECEIVE) &&
      reply[2] == UA && reply[3] == (reply[1] ^ reply[2]) && reply[4] == FLAG);
}

int is_frame_DISC(char *reply) {
  return (reply[0] == FLAG &&
          reply[1] ==
              ((data_link_layer.status == TRANSMITTER) ? RECEIVE : SEND) &&
          reply[2] == DISC && reply[3] == (reply[1] ^ reply[2]) &&
          reply[4] == FLAG);
}

// is_reply_valid is a function that checks if the reply received is valid.
// If it is, the send_frame ends correctly. If not, it continues its loop.
int send_frame(int fd, char *frame, int len, int (*is_reply_valid)(char *)) {
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
      if (is_reply_valid(reply))
        break;
    }
    printf("Connection failed. Retrying %d out of %d...\n", connection_timeouts,
           ACCEPTABLE_TIMEOUTS);
  }

  if (sigaction(SIGALRM, &old_action, NULL) == -1)
    printf("Error setting SIGALRM handler to original.\n");

  if (connection_timeouts == ACCEPTABLE_TIMEOUTS)
    return -1;
  else
    return 0;
}
