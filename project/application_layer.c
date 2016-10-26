#include "data_link_layer.h"

#define PACKET_START 2
#define PACKET_END 3

int set_up_connection(char *terminal, status stat) {
  if (stat != TRANSMITTER && stat != RECEIVER) {
    printf("application_layer :: set_up_connection() :: Invalid status.\n");
    return -1;
  }

  application.app_layer_status = stat;

  int port;
  if (strcmp(terminal, COM1_PORT) == 0)
    port = COM1;
  else if (strcmp(terminal, COM2_PORT) == 0)
    port = COM2;
  else {
    printf("application_layer :: set_up_connection() :: Invalid terminal\n");
  }

  if ((application.file_descriptor = ll_open(port, application.app_layer_status)) < 0)
  {
    printf("application_layer :: set_up_connection() :: ll_open failed\n");
    return -1;
  }

  return application.file_descriptor;
}

int send_data(char *data, int length) { return 0; }

int receive_data(char *data, int *length) {
  char packet[256];
  *length = 0;

  int packet_len;

  if(ll_read(application.file_descriptor, packet, &packet_len)!=0)
  {
    printf("Error ll_read() in function receive_data().\n");
    return -1;
  }

  printf("First packet: \n");

  print_as_hexadecimal(packet, packet_len);

  while (packet[0] != (unsigned char) PACKET_START)
  {
    ll_read(application.file_descriptor, packet, &packet_len);
  }

  printf("received start\n");

  while (packet[0] != (unsigned char) PACKET_END)
  {
    if(ll_read(application.file_descriptor, packet, &packet_len)!=0)
    {
      printf("Error ll_read() in function receive_data().\n");
      return -1;
    }
    // Lacks sequence number.
    int data_len = packet[2] * 256 + packet[3];
    memcpy(data + (*length), packet + 4, data_len);
    *length += data_len;
  }

  printf("received end\n");

  return 0;
}
