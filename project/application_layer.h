#ifndef APPLICATION_LAYER_H
#define APPLICATION_LAYER_H

#include "data_link_layer.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>

typedef struct {
  int file_descriptor;     /* Serial port file descriptor */
  status app_layer_status; /* TRANSMITTER | RECEIVER */
} app_layer;

app_layer application;

/**
 * Establishes a connection between the receiver and the transmitter
 * calls ll_open
 * returns -1 on error
 */
int set_up_connection(char *terminal, status stat);

/**
* Gets a full combination of packets and parses its information.
*/
int receive_data();

/**
* Writes file to serial port.
*/
int send_data(char *path, char *filename);

#endif
