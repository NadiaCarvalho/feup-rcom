#include <termios.h>

#define FLAG 0x7E

#define SEND 0x03
#define RECEIVE 0x01

#define SET 0x03
#define UA 0x07
#define DISC 0x0B
#define RR 0x05
#define REJ 0x01

typedef enum { TRANSMITTER, RECEIVER } Status;

//Debug functions
int read_from_tty(int fd, char *frame, int *frame_len);
int send_frame(int fd, char *frame, int len,
               int (*is_reply_valid)(char *));
