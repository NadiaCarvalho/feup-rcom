#ifndef __PARSE_URL_H__
#define  __PARSE_URL_H__

#define LINK_HEADER "ftp://"
#define DEFAULT_USER "anonymous"
#define DEFAULT_PASSWORD "mail@domain"

//se n existir user e pass - substituir por anonymous, mail@domain

typedef struct {
  char user[256];
  char password[256];
  char host_url[256];
  struct hostent* host_info;
  char file_path[256];
  char filename[256];
} url_info ;

/**
  * @brief Retrieves the username, password and download file name from a ftp URL
  * @param url URL with the information to parse_url
  * @param info Struct to complete with the information read from the URL
  * @returns 0 if URL is correct
  */
int parse_url(char url[], url_info* info);

#endif
