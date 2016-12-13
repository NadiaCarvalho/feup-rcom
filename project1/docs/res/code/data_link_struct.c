struct {
  char port[20]; /* Serial port device e.g. /dev/ttyS0 */
  int baud_rate;
  unsigned int sequence_num; /* Frame sequence number (0 or 1) */
  unsigned int timeout;      /* Time to timeout e.g. 1 second */
  unsigned int num_retries;  /* Maximum number of retries */
  status stat;
  struct sigaction old_action;
  int baudrate;
} data_link;
