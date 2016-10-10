#define BAUDRATE B9600
#define FALSE 0
#define TRUE 1
#define FLAG 0x7E
#define SEND 0x03
#define RECEIVE 0x01
#define SET 0x03
#define UA 0x07

#define TIMEOUT 3
#define RETRY 3


/**
* Write the message in buf, with buf_length length to fd.
*/
int send_message(int fd, char* buf, int buf_length);

/**
* Reads a message from fd and saves it in msg. msg_len is updated with the
* new string length.
*/
int read_message(int fd, char* msg, int* msg_len);

/**
* Prints msg with msg length to the screen in hexadecimal format.
*/
void print_as_hexadecimal(char *msg, int msg_len);

/**
* Write a message in fd, according to the control_bit given, using the *function send_message.
*/
int send_US_frame(int fd, int control_bit);
