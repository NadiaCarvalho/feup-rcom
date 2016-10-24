#include <termios.h>
#include "application_layer.h"

#define FLAG 0x7E

#define SEND 0x03
#define RECEIVE 0x01

#define SET 0x03
#define UA 0x07
#define DISC 0x0B
#define RR 0x05
#define REJ 0x01

typedef struct {
  char port[20];              /* Serial port device e.g. /dev/ttyS0 */
  int baud_rate;
  unsigned int sequence_num;  /* Frame sequence number (0 or 1) */
  unsigned int timeout;       /* Time to timeout e.g. 1 second */
  unsigned int num_retries;   /* Maximum number of retries */
} link_layer;

/**
 * Establishes a connection between the receiver and the transmitter
 */
int set_up_connection(char* port, app_layer application);

//Debug functions
int write_to_tty(int fd, char *buf, int buf_length);
int read_from_tty(int fd, char *frame, int *frame_len);
int send_frame(int fd, char *frame, int len,
               int (*is_reply_valid)(char *));


char *create_US_frame(int *frame_len, int control_bit);
int is_frame_UA(char *reply);
int is_frame_RR(char *reply);
int is_frame_DISC(char *reply);
