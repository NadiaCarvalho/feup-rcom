typedef struct {
  int file_descriptor;     /* Serial port file descriptor */
  status app_layer_status; /* TRANSMITTER | RECEIVER */
} app_layer;
