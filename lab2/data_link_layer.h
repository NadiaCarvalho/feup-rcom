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

/**
* Opens the terminal refered to by terminal.
* Updates the port settings and saves the old ones in
* old_port_settings.
* Depending on status, it send a SET or UA frame.
* Returns the according file descriptor on success,
* returning -1 otherwise.
*/
int ll_open(char* terminal, struct termios *old_port_settings, Status status);

/**
* Writes the given msg with len length to the
* given fd.
* Returns -1 on error.
*/
int ll_write(int fd, char* msg, int len);

/**
* Reads the message from fd and places it on
* msg, updating len accordingly.
* Returns -1 on error.
*/
int ll_read(int fd, char* msg, int* len);

/**
* Closes the given fd and sets the port settings.
* Returns -1 on error.
*/
int ll_close(int fd, struct termios *old_port_settings);

//Debug functions
int read_from_tty(int fd, char *frame, int *frame_len);
int send_frame(int fd, char *frame, int len,
               int (*is_reply_valid)(char *));
