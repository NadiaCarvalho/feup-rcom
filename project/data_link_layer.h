#ifndef DATA_LINK_LAYER_H
#define DATA_LINK_LAYER_H

#include "application_layer.h"
#include <termios.h>

#define FLAG 0x7E

#define SEND 0x03
#define RECEIVE 0x01

#define SET 0x03
#define UA 0x07
#define DISC 0x0B
#define RR 0x05
#define REJ 0x01

#define BAUDRATE B9600

#define COM1 0
#define COM2 1
#define COM1_PORT "/dev/ttyS0"
#define COM2_PORT "/dev/ttyS1"

#define US_FRAME_LENGTH 5

typedef struct {
  char port[20]; /* Serial port device e.g. /dev/ttyS0 */
  int baud_rate;
  unsigned int sequence_num; /* Frame sequence number (0 or 1) */
  unsigned int timeout;      /* Time to timeout e.g. 1 second */
  unsigned int num_retries;  /* Maximum number of retries */
} link_layer;

link_layer data_link;

/**
* Opens the terminal refered to by terminal.
* Updates the port settings and saves the old ones in
* old_port_settings.
* Depending on status, it send a SET or UA frame.
* Returns the according file descriptor on success,
* returning -1 otherwise.
*/
int ll_open(int port, status stat, struct termios *old_port_settings);

/**
* Writes the given msg with len length to the
* given fd.
* Returns -1 on error.
*/
int ll_write(int fd, char *msg, int len);

/**
* Reads the message from fd and places it on
* msg, updating len accordingly.
* Returns -1 on error.
*/
int ll_read(int fd, char *msg, int *len);

/**
* Closes the given fd and sets the port settings.
* Returns -1 on error.
*/
int ll_close(int fd, struct termios *old_port_settings);

// Debug functions
int write_to_tty(int fd, char *buf, int buf_length);
int read_from_tty(int fd, char *frame, int *frame_len);
int send_frame(int fd, char *frame, int len, int (*is_reply_valid)(char *));

char *create_I_frame(int *frame_len, char *packet, int packet_len);

char *create_US_frame(int *frame_len, int control_bit);
int is_frame_UA(char *reply);
int is_frame_RR(char *reply);
int is_frame_DISC(char *reply);

/**
* Change the terminal settings
* return -1 on error
*/
int set_terminal_attributes(int fd, struct termios *old_port_settings);

/**
* Stuffing the frame given.
*/
int stuffing(char *frame,int i,unsigned char type);

/**
* Destuffing the frame given.
*/
int destuffing(char* initialFrame,char *finalFrame,int initialFrameLenght);

#endif
