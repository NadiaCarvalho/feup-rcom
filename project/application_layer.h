#ifndef APPLICATION_LAYER_H
#define APPLICATION_LAYER_H

#include <unistd.h>
#include <stdio.h>
#include <string.h>

typedef enum { TRANSMITTER, RECEIVER } status;

typedef struct {
  int file_descriptor;  /* Serial port file descriptor */
  status app_layer_status; /* TRANSMITTER | RECEIVER */
} app_layer;

app_layer application;
/**
* Change the terminal settings
* return -1 on error
*/
int set_terminal_attributes(struct termios *old_port_settings, struct termios *new_port_settings);

/**
 * Establishes a connection between the receiver and the transmitter
 * calls ll_open
 * returns -1 on error
 */
int set_up_connection(char* terminal, struct termios *oldtio, status stat);

#endif
