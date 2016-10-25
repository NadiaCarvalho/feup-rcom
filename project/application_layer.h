#ifndef APPLICATION_LAYER_H
#define APPLICATION_LAYER_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef enum { TRANSMITTER, RECEIVER } status;

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
int receive_data(char *data, int *length);

#endif
