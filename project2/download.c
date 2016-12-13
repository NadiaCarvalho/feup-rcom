#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include "parse_url.h"
#include "tcp.h"


int main(int argc, char** argv){

  if(argc != 2){
    fprintf(stderr, "Usage: %s <address>\n", argv[0]);
    exit(1);
  }

  url_info info;
  if(parse_url(argv[1], &info) != 0){
    fprintf(stderr, "Invalid URL\n");
    exit(1);
  }

  int control_socket_fd;
  if((control_socket_fd = create_connection(inet_ntoa(*((struct in_addr *)info.host_info->h_addr)), CLIENT_CONNECTION_PORT)) == 0){
    fprintf(stderr, "Error opening control connection\n");
    exit(1);
  }

  login(control_socket_fd, &info);
  char data_address[MAX_STRING_SIZE];
  int port;
  enter_passive_mode(control_socket_fd, data_address, &port);

  int data_socket_fd;
  if((data_socket_fd = create_connection(data_address, port)) == 0){
    fprintf(stderr, "Error opening data connection\n");
    exit(1);
  }
  send_retrieve(control_socket_fd, &info);
  download_file(data_socket_fd, &info);
  close_connection(control_socket_fd, data_socket_fd);

  return 0;
}
