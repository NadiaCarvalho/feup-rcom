#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  char* at_position = strchr(url, '@');
  if(at_position == NULL)
    initialize_default_auth(info);
  else{
    if(initialize_auth(info, url, at_position) != 0)
      return 1;
  }
  char* last_slash = strrchr(url, '/');
  last_slash++;
  memcpy(info->filename, last_slash, strlen(last_slash) + 1);
  return 0;
}
