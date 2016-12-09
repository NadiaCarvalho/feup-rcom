#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include "parse_url.h"

#define CLIENT_CONNECTION_PORT 21
#define MAX_STRING_SIZE 256


int create_connection(char* address, int port){
  int	sockfd;
	struct	sockaddr_in server_addr;

	/*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(address);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port);		/*server TCP port must be network byte ordered */

	/*open an TCP socket*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    		perror("socket()");
        	return 0;
    	}
	/*connect to the server*/
    	if(connect(sockfd,
	           (struct sockaddr *)&server_addr,
		   sizeof(server_addr)) < 0){
        	perror("connect()");
		return 0;
	}
  return sockfd;
}

int read_from_socket(int sockfd, char* str){
  int allocated = 0;
  if(str == NULL){
    str = (char*) malloc(sizeof(char) * MAX_STRING_SIZE);
    allocated = 1;
  }
  do {
    memset(str, 0, MAX_STRING_SIZE);
    read(sockfd, str, MAX_STRING_SIZE);
    printf("%s", str);
  } while (str[0] == '4');
  char reply_series = str[0];
  if(allocated)
    free(str);
  return (reply_series > '4');
}

int write_to_socket(int sockfd, char* cmd, char* response){
    write(sockfd, cmd, strlen(cmd));
    return read_from_socket(sockfd, response);
}

void login(int sockfd, url_info* info){

  char username_cmd[MAX_STRING_SIZE], password_cmd[MAX_STRING_SIZE];

  sprintf(username_cmd, "USER %s\r\n", info->user);
  write_to_socket(sockfd, username_cmd, NULL);
  sprintf(password_cmd, "PASS %s\r\n", info->password);
  write_to_socket(sockfd, password_cmd, NULL);
}

void enter_passive_mode(int sockfd, char* ip, int* port){
  char response[MAX_STRING_SIZE];

  write_to_socket(sockfd, "PASV\r\n", response);

  int values[6];
  char* data = strchr(response, '(');
  sscanf(data, "(%d, %d, %d, %d, %d, %d)", &values[0],&values[1],&values[2],&values[3],&values[4],&values[5]);
  sprintf(ip, "%d.%d.%d.%d", values[0],values[1],values[2],values[3]);
  *port = values[4]*256+values[5];
}

void send_retrieve(int control_socket_fd, int data_socket_fd, url_info* info){
  char cmd[MAX_STRING_SIZE];

  sprintf(cmd, "RETR %s%s\r\n", info->file_path, info->filename);
  write_to_socket(control_socket_fd, cmd, NULL);
}

int download_file(int data_socket_fd, url_info* info){
  FILE* outfile = fopen(info->filename, "w");

  char buf[1024];
  int bytes;
  while ((bytes = read(data_socket_fd, buf, sizeof(buf)))) {
    if (bytes < 0) {
      fprintf(stderr, "ERROR: Nothing was received from data socket fd.\n");
      return 1;
    }

    if ((bytes = fwrite(buf, bytes, 1, outfile)) < 0) {
      fprintf(stderr, "ERROR: Cannot write data in file.\n");
      return 1;
    }
  }

  fclose(outfile);

  return 0;
}

int close_connection(int control_socket_fd, int data_socket_fd){

  read_from_socket(control_socket_fd, NULL);
  write_to_socket(control_socket_fd, "QUIT\r\n", NULL);

  close(data_socket_fd);
  close(control_socket_fd);

  return 0;
}

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

  read_from_socket(control_socket_fd, NULL);
  login(control_socket_fd, &info);
  char data_address[MAX_STRING_SIZE];
  int port;
  enter_passive_mode(control_socket_fd, data_address, &port);

  int data_socket_fd;
  if((data_socket_fd = create_connection(data_address, port)) == 0){
    fprintf(stderr, "Error opening data connection\n");
    exit(1);
  }
  send_retrieve(control_socket_fd, data_socket_fd, &info);
  download_file(data_socket_fd, &info);
  close_connection(control_socket_fd, data_socket_fd);

}
