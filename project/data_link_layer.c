#include "data_link_layer.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <termios.h>
#include <unistd.h>

#define BAUDRATE B9600
#define ACCEPTABLE_TIMEOUTS 3

volatile int STOP = 0;
struct termios old_port_settings;
int connection_timeouts = 0;

struct {
  char port[20]; /* Serial port device e.g. /dev/ttyS0 */
  int baud_rate;
  unsigned int sequence_num; /* Frame sequence number (0 or 1) */
  unsigned int timeout;      /* Time to timeout e.g. 1 second */
  unsigned int num_retries;  /* Maximum number of retries */
  status stat;
  struct sigaction old_action;
} data_link;

//'Private' functions
int write_to_tty(int fd, char *buf, int buf_length);
int read_from_tty(int fd, char *frame, int *frame_len);
int send_frame(int fd, char *frame, int len, int (*is_reply_valid)(char *));
char *create_I_frame(int *frame_len, char *packet, int packet_len);
char *create_US_frame(int *frame_len, int control_byte);
int is_frame_UA(char *reply);
int is_frame_RR(char *reply);
int is_frame_DISC(char *reply);
void timeout(int signum);

/**
* Change the terminal settings
* return -1 on error
*/
int set_terminal_attributes(int fd);
/**
* Stuffing the frame given.
*/
char *stuff(char *packet, int *packet_len);
/**
* Destuffing the frame given.
*/
void destuff(char *packet, char *destuffed, int *packet_len);

/*
* Definitions
*/

int set_terminal_attributes(int fd) {
  struct termios new_port_settings;

  if (tcgetattr(fd, &old_port_settings) ==
      -1) { /* save current port settings */
    printf("Error getting port settings.\n");
    close(fd);
    return -1;
  }

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

  return 0;
}

// Checks are not full. Need to check BCC of the packet.
int is_I_frame_valid(char *frame, int frame_len, int seq_num) {

  if (frame_len < 6)
    return 0;

  if (frame[0] != FLAG || frame[1] != SEND || frame[2] != seq_num ||
      frame[3] != (frame[1] ^ frame[2]))
    return 0;

  return 1;
}

/**
 * TODO:
 * if stat == TRANSMITTER -> send SET, receive UA
 * reverse if stat == RECEIVE
 */
int ll_open(int port, status stat) {

  int fd; // value to be returned
  int frame_len;

  switch (port) {
  case COM1:
    strcpy(data_link.port, COM1_PORT);
    break;

  case COM2:
    strcpy(data_link.port, COM2_PORT);
    break;

  default:
    printf("data_link_layer :: ll_open() :: invalid port!\n");
    return -1;
  }

  if (stat != TRANSMITTER && stat != RECEIVER) {
    printf("data_link_layer :: ll_open() :: Invalid status.\n");
    return -1;
  }

  data_link.stat = stat;
  /**
   * Opening the serial port
   */
  if ((fd = open(data_link.port, O_RDWR | O_NOCTTY)) < 0) {
    printf("Error opening terminal '%s'\n", data_link.port);
    return -1;
  }

  if (set_terminal_attributes(fd) != 0) {
    printf("Error set_terminal_attributes() in function ll_open().\n");
    return -1;
  }

  struct sigaction new_action;
  new_action.sa_handler = timeout;
  new_action.sa_flags &=
      !SA_RESTART; // Needed in order to block read from restarting.

  if (sigaction(SIGALRM, &new_action, &data_link.old_action) == -1)
    printf("Error installing new SIGALRM handler.\n");

  if (stat == TRANSMITTER) {

    char *frame = create_US_frame(&frame_len, SET);
    if (send_frame(fd, frame, frame_len, is_frame_UA) == -1) {
      printf("data_link_layer :: ll_open() :: send_frame failed\n");
      close(fd);
      return -1;
    }

  } else {
    char msg[255];
    int msg_len;
    if (read_from_tty(fd, msg, &msg_len) == -1) {
      printf("Error read_from_tty() in function ll_open().\n");
      return -1;
    }

    char *frame = create_US_frame(&frame_len, UA);
    if (write_to_tty(fd, frame, frame_len) == -1) {
      printf("Error write_to_tty() in function ll_open().\n");
      return -1;
    }
  }

  printf("data_link_layer :: ll_open() :: connection succesfully "
         "established.\n");

  return fd;
}

int ll_write(int fd, char *packet, int packet_len) {
  // Writes and checks for validity
  // Using send_frame

  packet = stuff(packet, &packet_len);

  int frame_len;
  char *frame = create_I_frame(&frame_len, packet, packet_len);
  send_frame(fd, frame, frame_len, is_frame_RR);

  return 0;
}

int ll_read(int fd, char *packet, int *len) {

  // Reads and checks for validity
  int reply_len;
  static int seq_num = 0;

  char frame[256];
  int frame_len;
  read_from_tty(fd, frame, &frame_len);

  /*if (!is_I_frame_valid(frame, frame_len, seq_num)) {
    return -1;
  }*/

  // Updates the packet length.
  *len = frame_len - 6;
  destuff(frame + 4, packet, len);

  char *reply = create_US_frame(&reply_len, RR);

  if (write_to_tty(fd, reply, reply_len) != 0) {
    printf("Error write_to_tty() in function ll_read().\n");
    return -1;
  }

  return 0;
}

int ll_close(int fd) {
  char *frame;
  int frame_len = 0;

  if (data_link.stat == TRANSMITTER) {
    frame = create_US_frame(&frame_len, DISC);
    if (send_frame(fd, frame, frame_len, is_frame_DISC) != 0) {
      printf("Couldn't send frame on ll_close().\n");
      return -1;
    }

    if (write_to_tty(fd, create_US_frame(&frame_len, UA), frame_len) != 0) {
      printf("Couldn't write to tty on ll_close()\n");
      return -1;
    }

  } else {
    char msg[256];
    int msg_len = 0;

    if (read_from_tty(fd, msg, &msg_len) != 0) {
      printf("Couldn't read from tty on ll_close()\n");
      return -1;
    }

    if (is_frame_DISC(msg)) {
      frame = create_US_frame(&frame_len, DISC);
      if (send_frame(fd, frame, frame_len, is_frame_UA) != 0) {
        printf("Couldn't send frame on ll_close().\n");
        return -1;
      }
    }
  }

  if (sigaction(SIGALRM, &data_link.old_action, NULL) == -1)
    printf("Error setting SIGALRM handler to original.\n");

  if (tcsetattr(fd, TCSANOW, &old_port_settings) == -1)
    printf("Error settings old port settings.\n");

  if (close(fd)) {
    printf("Error closing terminal file descriptor.\n");
    return -1;
  }

  printf("Connection succesfully closed.\n");

  return 0;
}

void print_as_hexadecimal(char *msg, int msg_len) {
  int i;
  for (i = 0; i < msg_len; i++)
    printf("%02X ", msg[i]);
  fflush(stdout);
}

void timeout(int signum) { connection_timeouts++; }

char *create_US_frame(int *frame_len, int control_byte) {
  static char r = 0;
  char *buf = (char *)malloc(US_FRAME_LENGTH * sizeof(char));
  buf[0] = FLAG;

  if (data_link.stat == TRANSMITTER) {
    if (control_byte == SET || control_byte == DISC)
      buf[1] = SEND;
    else
      buf[1] = RECEIVE;
  } else {
    if (control_byte == RR || control_byte == REJ || control_byte == UA)
      buf[1] = SEND;
    else
      buf[1] = RECEIVE;
  }

  if (control_byte == RR || control_byte == REJ) {
    buf[2] = r << 7 | control_byte;
    r = !r;
  } else
    buf[2] = control_byte;

  buf[3] = buf[1] ^ buf[2];
  buf[4] = FLAG;
  *frame_len = US_FRAME_LENGTH;

  return buf;
}

int write_to_tty(int fd, char *buf, int buf_length) {
  int total_written_chars = 0;
  int written_chars = 0;

  while (total_written_chars < buf_length) {
    written_chars = write(fd, buf, buf_length);

    if (written_chars <= 0) {
      printf("Written chars: %d\n", written_chars);
      printf("%s\n", strerror(errno));
      return -1;
    }

    total_written_chars += written_chars;
  }

  return 0;
}

int read_from_tty(int fd, char *frame, int *frame_len) {

  int read_chars = 0;
  char buf;
  *frame_len = 0;
  int initial_flag = 0;

  STOP = 0;
  while (!STOP) {                   /* loop for input */
    read_chars = read(fd, &buf, 1); /* returns after x chars have been input */

    if (read_chars == 0) {
      printf("EOF.\n");
      return -1;
    }

    if (read_chars > 0) { // If characters were read
      if (buf == FLAG) {

        initial_flag = (initial_flag + 1) % 2;

        if (!initial_flag)
          STOP = 1;
      }

      frame[*frame_len] = buf;
      (*frame_len)++;

    } else // If no characters were read or there was an error
      return -1;
  }

  return 0;
}

int is_frame_UA(char *reply) {

  return (reply[0] == FLAG &&
          reply[1] == ((data_link.stat == TRANSMITTER) ? SEND : RECEIVE) &&
          reply[2] == UA && reply[3] == (reply[1] ^ reply[2]) &&
          reply[4] == FLAG);
}

int is_frame_DISC(char *reply) {
  return (reply[0] == FLAG &&
          reply[1] == ((data_link.stat == TRANSMITTER) ? RECEIVE : SEND) &&
          reply[2] == DISC && reply[3] == (reply[1] ^ reply[2]) &&
          reply[4] == FLAG);
}

// is_reply_valid is a function that checks if the reply received is valid.
// If it is, the send_frame ends correctly. If not, it continues its loop.
int send_frame(int fd, char *frame, int len, int (*is_reply_valid)(char *)) {
  char reply[255];
  int reply_len;

  connection_timeouts = 0;
  while (connection_timeouts < ACCEPTABLE_TIMEOUTS) {
    if (write_to_tty(fd, frame, len)) {
      printf("Failed write.\n");
      return -1;
    }

    alarm(3);

    if (read_from_tty(fd, reply, &reply_len) == 0) {
      // If the read() was successful
      alarm(0);
      if (is_reply_valid(reply))
        break;

    } else {
      printf("Error reading.\n");
      alarm(0);
      return -1;
    }

    if (connection_timeouts > 0)
      printf("Connection failed. Retrying %d out of %d...\n",
             connection_timeouts, ACCEPTABLE_TIMEOUTS);
  }

  alarm(0);
  if (connection_timeouts == ACCEPTABLE_TIMEOUTS)
    return -1;
  else
    return 0;
}

int is_frame_RR(char *reply) {
  static char r = 1;
  r = !r;
  return (
      reply[0] == (unsigned char)FLAG &&
      reply[1] ==
          (unsigned char)((data_link.stat == TRANSMITTER) ? SEND : RECEIVE) &&
      (unsigned char)reply[2] == (unsigned char)(r << 7 | RR) &&
      (unsigned char)reply[3] == (unsigned char)(reply[1] ^ reply[2]) &&
      reply[4] == (unsigned char)FLAG);
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

char *stuff(char *packet, int *packet_len) {
  // TODO: Variable size.
  char *stuffed = (char *)malloc(((*packet_len) + 255) * sizeof(char));

  int packet_iterator;
  int stuff_iterator = 0;

  for (packet_iterator = 0; packet_iterator < *packet_len; packet_iterator++) {

    if (packet[packet_iterator] == ESCAPE || packet[packet_iterator] == FLAG) {

      stuffed[stuff_iterator] = ESCAPE;
      stuffed[++stuff_iterator] = packet[packet_iterator] ^ STUFFING_BYTE;
    } else
      stuffed[stuff_iterator] = packet[packet_iterator];

    stuff_iterator++;
  }

  *packet_len = stuff_iterator;
  return stuffed;
}

void destuff(char *packet, char *destuffed, int *packet_len) {
  int destuff_iterator = 0;

  for (int i = 0; i < *packet_len; i++) {
    if (packet[i] == ESCAPE) {
      destuffed[destuff_iterator] = packet[++i] ^ STUFFING_BYTE;
    } else
      destuffed[destuff_iterator] = packet[i];

    destuff_iterator++;
  }

  *packet_len = destuff_iterator;
}
