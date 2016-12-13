#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include "tcp.h"

#define READ 1
#define NO_READ 0

int read_from_socket(int sockfd, char* str){
  	FILE* fp = fdopen(sockfd, "r");
  int allocated = 0;
  if(str == NULL){
    str = (char*) malloc(sizeof(char) * MAX_STRING_SIZE);
    allocated = 1;
  }
  do {
    memset(str, 0, MAX_STRING_SIZE);
    str = fgets(str, MAX_STRING_SIZE, fp);
    printf("%s", str);
}  while (!('1' <= str[0] && str[0] <= '5') || str[3] != ' ');
  char reply_series = str[0];
  if(allocated)
    free(str);
  return (reply_series > '4');
}

int write_to_socket(int sockfd, char* cmd, char* response, int read){
    int return_val = write(sockfd, cmd, strlen(cmd));
    if(read)
      return read_from_socket(sockfd, response);
    else return (return_val == 0);
}

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

void login(int control_socket_fd, url_info* info){

  char username_cmd[MAX_STRING_SIZE], password_cmd[MAX_STRING_SIZE];

  read_from_socket(control_socket_fd, NULL);

  sprintf(username_cmd, "USER %s\r\n", info->user);
  write_to_socket(control_socket_fd, username_cmd, NULL, READ);
  sprintf(password_cmd, "PASS %s\r\n", info->password);
  if(write_to_socket(control_socket_fd, password_cmd, NULL, READ) != 0){
      fprintf(stderr, "Bad login. Exiting...\n"); //TODO: Ask for valid login
      exit(1);
  }
}

void enter_passive_mode(int sockfd, char* ip, int* port){
  char response[MAX_STRING_SIZE];

  if(write_to_socket(sockfd, "PASV\r\n", response, READ) != 0){
    fprintf(stderr, "Error entering passive mode. Exiting...\n");
    exit(1);
  }

  int values[6];
  char* data = strchr(response, '(');
  sscanf(data, "(%d, %d, %d, %d, %d, %d)", &values[0],&values[1],&values[2],&values[3],&values[4],&values[5]);
  sprintf(ip, "%d.%d.%d.%d", values[0],values[1],values[2],values[3]);
  *port = values[4]*256+values[5];
}

void send_retrieve(int control_socket_fd, url_info* info){
  char cmd[MAX_STRING_SIZE];

  write_to_socket(control_socket_fd, "TYPE L 8\r\n", NULL, READ);
  sprintf(cmd, "RETR %s%s\r\n", info->file_path, info->filename);
  if(write_to_socket(control_socket_fd, cmd, NULL, READ) != 0){
    fprintf(stderr, "Error retrieving file. Exiting...\n");
    exit(1);
  }
}

int download_file(int data_socket_fd, url_info* info){
  FILE* outfile;
  if(!(outfile = fopen(info->filename, "w"))) {
		printf("ERROR: Cannot open file.\n");
		return 1;
	}

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

  printf("Finished downloading file\n");

  return 0;
}

int close_connection(int control_socket_fd, int data_socket_fd){

  printf("Closing connection\n");
  if(write_to_socket(control_socket_fd, "QUIT\r\n", NULL, NO_READ) != 0){
    fprintf(stderr, "Error closing connection. Exiting anyway...\n");
    close(data_socket_fd);
    close(control_socket_fd);
    exit(1);
  }

  close(data_socket_fd);
  close(control_socket_fd);

  printf("Goodbye!\n");

  return 0;
}
