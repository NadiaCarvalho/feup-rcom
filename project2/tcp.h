#include "parse_url.h"

#define CLIENT_CONNECTION_PORT 21
#define MAX_STRING_SIZE 256

int create_connection(char* address, int port);

void login(int control_socket_fd, url_info* info);

void enter_passive_mode(int sockfd, char* ip, int* port);

void send_retrieve(int control_socket_fd, int data_socket_fd, url_info* info);

int download_file(int data_socket_fd, url_info* info);

int close_connection(int control_socket_fd, int data_socket_fd);
