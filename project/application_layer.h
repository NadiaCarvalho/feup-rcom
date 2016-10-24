#include <stdio.h>

typedef enum { TRANSMITTER, RECEIVER } status;

typedef struct {
  int file_descriptor;  /* Serial port file descriptor */
  status app_layer_status; /* TRANSMITTER | RECEIVER */
} app_layer;

/**
* Opens the terminal refered to by terminal.
* Updates the port settings and saves the old ones in
* old_port_settings.
* Depending on status, it send a SET or UA frame.
* Returns the according file descriptor on success,
* returning -1 otherwise.
*/
int ll_open(char* terminal, struct termios *old_port_settings, status app_layer_status);

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

/**
* Change the terminal settings
* return -1 on error
*/
int set_terminal_attributes(struct termios *old_port_settings);
