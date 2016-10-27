#include "application_layer.h"

#define DATA_PACKET_BYTE 1
#define START_PACKET_BYTE 2
#define END_PACKET_BYTE 3

#define FILE_SIZE_BYTE 0
#define FILE_NAME_BYTE 1
#define FILE_PERMISSIONS_BYTE 2

#define PACKET_SIZE 256
#define PACKET_HEADER_SIZE 4
#define PACKET_DATA_SIZE PACKET_SIZE - PACKET_HEADER_SIZE

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

  if ((application.file_descriptor =
           ll_open(port, application.app_layer_status)) < 0) {
    printf("application_layer :: set_up_connection() :: ll_open failed\n");
    return -1;
  }

  return application.file_descriptor;
}

int get_file_size(char *packet, int packet_len) {
  int i = 1;
  while (i < packet_len) {
    if (packet[i] == FILE_SIZE_BYTE)
      return *((int *)(packet + i + 2));

    i += 2 + packet[i + 1];
  }

  return 0;
}

char *get_file_name(char *packet, int packet_len) {
  int i = 1;
  printf("%s\n", packet + 5);
  while (i < packet_len) {
    if (packet[i] == FILE_NAME_BYTE) {
      char *file_name = (char *)malloc((packet[i + 1] + 1) * sizeof(char));
      memcpy(file_name, packet + i + 2, packet[i + 1]);
      file_name[(packet[i + 1] + 1)] = 0;
      return file_name;
    }

    i += 2 + packet[i + 1];
  }

  return NULL;
}

int get_file_permissions() {}

int send_data(char *path, char *filename) {
  char *full_path =
      (char *)malloc(sizeof(char) * (strlen(path) + 1 + strlen(filename)));
  strcpy(full_path, path);
  strcat(full_path, "/");
  strcat(full_path, filename);
  int fd = open(full_path, O_RDONLY);
  // free(full_path);

  if (fd < 0) {
    printf("Error opening file. Exiting...\n");
    return -1;
  }

  struct stat file_info;
  fstat(fd, &file_info);

  int filename_len = strlen(filename);
  off_t file_size = file_info.st_size;

  /*
  * START PACKET
  */
  int start_packet_len = 5 + sizeof(file_info.st_size) + filename_len;
  char *start_packet = (char *)malloc(sizeof(char) * start_packet_len);
  start_packet[0] = START_PACKET_BYTE;
  start_packet[1] = FILE_SIZE_BYTE;
  start_packet[2] = sizeof(file_info.st_size);
  *((off_t *)(start_packet + 3)) = file_size;
  start_packet[3 + sizeof(file_info.st_size)] = FILE_NAME_BYTE;
  start_packet[4 + sizeof(file_info.st_size)] = filename_len;
  strcat(start_packet + 5 + sizeof(file_info.st_size), filename);
  ll_write(application.file_descriptor, start_packet, start_packet_len);
  /*
  * DATA PACKET
  */
  char data[PACKET_DATA_SIZE];
  int i;
  off_t bytes_remaining = file_size;

  for (i = 0; i <= file_size / PACKET_DATA_SIZE; i++) {
    if (read(application.file_descriptor, data, PACKET_DATA_SIZE) <= 0) {
      printf("Error reading from file. Exiting...\n");
      return -1;
    }

    char information_packet[PACKET_SIZE];
    information_packet[0] = DATA_PACKET_BYTE;
    information_packet[1] = i % 256;
    information_packet[2] = (bytes_remaining < PACKET_DATA_SIZE)
                                ? (PACKET_HEADER_SIZE + bytes_remaining) % 256
                                : PACKET_SIZE % 256;
    information_packet[3] = (bytes_remaining < PACKET_DATA_SIZE)
                                ? (PACKET_HEADER_SIZE + bytes_remaining) / 256
                                : PACKET_SIZE / 256;

    memcpy(information_packet + PACKET_HEADER_SIZE, data, PACKET_DATA_SIZE);
    ll_write(application.file_descriptor, information_packet, PACKET_SIZE);
    bytes_remaining -= PACKET_DATA_SIZE;
  }

  /*
  * END PACKET
  */
  char end_packet[] = {3};
  ll_write(fd, end_packet, 1);
  exit(1);
  return 0;
}

int receive_data() {
  char packet[PACKET_SIZE];

  int packet_len;
  /**
  * Reading and parsing start packet
  */
  do {
    if (ll_read(application.file_descriptor, packet, &packet_len) != 0) {
      printf("Error ll_read() in function receive_data().\n");
      return -1;
    }
  } while (packet[0] != (unsigned char)START_PACKET_BYTE);

  int file_size = get_file_size(packet, packet_len);
  char *file_name = get_file_name(packet, packet_len);

  char temp_fn[strlen(file_name) + 4];
  strcpy(temp_fn, file_name);
  strcat(temp_fn, ".gif");
  int fd = open(temp_fn, O_WRONLY | O_CREAT);
  // int fd = open(file_name, O_WRONLY | O_CREAT);

  if (fd < 0) {
    printf("Error opening file. Exiting...\n");
    return -1;
  }

  printf("Reading data packets...\n");
  /**
  * Reading and parsing data packets
  */
  if (ll_read(application.file_descriptor, packet, &packet_len) != 0) {
    printf("Error ll_read() in function receive_data().\n");
    close(fd);
    return -1;
  }

  while (packet[0] != (unsigned char)END_PACKET_BYTE) {
    // Lacks sequence number.
    int data_len = packet[2] * 256 + packet[3];
    write(fd, packet + 4, data_len);

    if (ll_read(application.file_descriptor, packet, &packet_len) != 0) {
      printf("Error ll_read() in function receive_data().\n");
      close(fd);
      return -1;
    }
  }

  close(fd);
  return 0;
}
