#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "parse_url.h"

void initialize_default_auth(url_info* info){
  memcpy(info->user, DEFAULT_USER, strlen(DEFAULT_USER) + 1);
  memcpy(info->password, DEFAULT_PASSWORD, strlen(DEFAULT_PASSWORD) + 1);
};

int initialize_auth(url_info* info, char* url, char* at_position){
  char* slash = strchr(url, '/'); //slash is never null
  slash += 2;
  char* password = strchr(slash, ':');
  if(password == NULL){
    fprintf(stderr, "Your link must contain a ':' separating the username and password!'\n");
    return 1;
  }
  memcpy(info->user, slash, password - slash);
  info->user[password-slash]=0;
  password++;
  memcpy(info->password, password, at_position-password);
  info->password[at_position-password] = 0;
  return 0;
}

int parse_url(char url[], url_info* info){
  if(strncmp(url, LINK_HEADER, strlen(LINK_HEADER)) != 0){
    fprintf(stderr, "Your link must begin with 'ftp://'\n");
    return 1;
  }
  char* at_position = strrchr(url, '@');
  if(at_position == NULL){
    initialize_default_auth(info);
    at_position = url + strlen("ftp://");
  }
  else{
    if(initialize_auth(info, url, at_position) != 0)
      return 1;
    at_position++;
  }

  char* first_slash = strchr(at_position, '/');
  memcpy(info->host_url, at_position, first_slash-at_position);
  info->host_url[first_slash-at_position] = 0;

  char* last_slash = strrchr(url, '/');
  last_slash++;
  memcpy(info->file_path, first_slash, last_slash-first_slash);
  info->file_path[last_slash-first_slash] = 0;

  memcpy(info->filename, last_slash, strlen(last_slash) + 1);
  if ((info->host_info=gethostbyname(info->host_url)) == NULL) {
      herror(info->host_url);
      exit(1);
  }

  return 0;
}

// int main(int argc, char* argv[]){
//   url_info info;
//   parse_url(argv[1], &info);
//   return 0;
// }
