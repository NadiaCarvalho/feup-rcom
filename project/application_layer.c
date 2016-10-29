#include "application_layer.h"

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

off_t get_file_size(char *packet, int packet_len) {
  int i = 1;
  while (i < packet_len) {
    if (packet[i] == FILE_SIZE_BYTE)
      return *((off_t *)(packet + i + 2));

    i += 2 + packet[i + 1];
  }

  return 0;
}

char *get_file_name(char *packet, int packet_len) {
  int i = 1;
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

mode_t get_file_permissions(char *packet, int packet_len) {
  int i = 1;
  while (i < packet_len) {
    if (packet[i] == FILE_PERMISSIONS_BYTE)
      /*
      * mode_t is 4 bytes long, but the information sent is
      * only 2 bytes. As such, we need to read it as a 2 byte int
      * and cast it to mode_t.
      */
      return *((uint16_t *)(packet + i + 2));

    i += 2 + packet[i + 1];
  }

  return -1;
}

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
  mode_t file_mode = file_info.st_mode;

  /*
  * START PACKET
  */
  int start_packet_len = 9 + sizeof(file_info.st_size) + filename_len;
  char *start_packet = (char *)malloc(sizeof(char) * start_packet_len);
  start_packet[0] = START_PACKET_BYTE;
  start_packet[1] = FILE_PERMISSIONS_BYTE;
  start_packet[2] = 2;
  *((mode_t *)(start_packet + 3)) = file_mode;

  start_packet[5] = FILE_SIZE_BYTE;
  start_packet[6] = sizeof(file_info.st_size);
  *((off_t *)(start_packet + 7)) = file_size;
  start_packet[7 + sizeof(file_info.st_size)] = FILE_NAME_BYTE;
  start_packet[8 + sizeof(file_info.st_size)] = filename_len;
  strcat(start_packet + 9 + sizeof(file_info.st_size), filename);
  ll_write(application.file_descriptor, start_packet, start_packet_len);

  /*
  * DATA PACKET
  */
  char data[PACKET_DATA_SIZE];
  int i = 0;
  off_t bytes_remaining = file_size;

  while (bytes_remaining > 0) {
    int read_chars;
    if ((read_chars = read(fd, data, PACKET_DATA_SIZE)) <= 0) {
      printf("Error reading from file. Exiting...\n");
      return -1;
    }

    char information_packet[PACKET_SIZE];
    int packet_size = read_chars + PACKET_HEADER_SIZE;
    information_packet[0] = DATA_PACKET_BYTE;
    information_packet[1] = i % 256;
    information_packet[2] = read_chars / 256;
    information_packet[3] = read_chars % 256;

    memcpy(information_packet + PACKET_HEADER_SIZE, data, read_chars);
    ll_write(application.file_descriptor, information_packet, packet_size);
    bytes_remaining -= read_chars;
    i++;
  }

  /*
  * END PACKET
  */
  char end_packet[] = {3};
  ll_write(application.file_descriptor, end_packet, 1);
  close(fd);
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
  } while (packet_len == 0 || packet[0] != (unsigned char)START_PACKET_BYTE);

  off_t file_size = get_file_size(packet, packet_len);
  char *file_name = get_file_name(packet, packet_len);
  mode_t file_mode = get_file_permissions(packet, packet_len);

  // FIXME: Needed to avoid overwriting current file.
  // Must be removed afterwards.
  char temp_fn[strlen(file_name) + 4];
  strcpy(temp_fn, file_name);
  strcat(temp_fn, ".gif");
  int fd = open(temp_fn, O_WRONLY | O_CREAT | O_TRUNC, file_mode);
  // int fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, file_mode);

  if (fd < 0) {
    printf("Error opening file. Exiting...\n");
    return -1;
  }

  /**
  * Reading and parsing data packets
  */
  char cur_seq_num = 0;
  if (ll_read(application.file_descriptor, packet, &packet_len) != 0) {
    printf("Error ll_read() in function receive_data().\n");
    close(fd);
    return -1;
  }

  while (packet_len == 0 || packet[0] != (unsigned char)END_PACKET_BYTE) {
    if (packet_len > 0 && cur_seq_num == packet[1]) {
      unsigned int data_len =
          (unsigned char)packet[2] * 256 + (unsigned char)packet[3];
      write(fd, packet + 4, data_len);
      cur_seq_num++;
    }

    if (ll_read(application.file_descriptor, packet, &packet_len) != 0) {
      printf("Error ll_read() in function receive_data().\n");
      close(fd);
      return -1;
    }
  }

  struct stat file_info;
  fstat(fd, &file_info);
  printf("Expected %lu bytes.\nReceived %lu bytes.\n", file_size,
         file_info.st_size);

  close(fd);
  return 0;
}
