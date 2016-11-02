#include "data_link_layer.h"
#include "application_layer.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

void select_parameters(){

  int temp_num_transmissions;
  int num_transmissions_flag=0;
  int temp_time_out;
  int time_out_flag=0;
  int chosen_baudrate;
  int baudrate_flag = 0;
  int baudrate;

  printf("Choose the number of times you want to retry on timeout: ");

  do{

    scanf("%d",&temp_num_transmissions);

    if(temp_num_transmissions > 0 && temp_num_transmissions<=6){
      printf("Number of retries will be %d.\n",temp_num_transmissions);
      num_transmissions_flag=1;
    }
    else{
      printf("\nInvalid value, choose again [1,...,6]: ");
    }


  }while(!num_transmissions_flag);


  printf("Choose the time for timeout: ");

  do{

    scanf("%d",&temp_time_out);

    if(temp_time_out > 0 && temp_time_out <=6){
      printf("Timeout between the transmissions will be %d.\n",temp_time_out);
      time_out_flag=1;
    }
    else
    {
      printf("\nInvalid value, choose again [1,...,6]: ");
    }

  }while(!time_out_flag);


  printf("Choose Baud Rate:\n1 - B4800\n2 - B9600\n3 - B19200\n\n");
  do{

    scanf(" %d",&chosen_baudrate);

    if(chosen_baudrate > 0 && chosen_baudrate <= 3){
      baudrate_flag=1;
    }
    else
    {
      printf("\nInvalid value, choose again [1,...,6]: ");
    }

  }while(!baudrate_flag);

  switch(chosen_baudrate){
    case 1:
      baudrate = B2400;
      printf("Baudrate: B2400\n");
      break;

    case 2:
      baudrate = B4800;
      printf("Baudrate: B4800\n");
      break;

    case 3:
      baudrate = B9600;
      printf("Baudrate: B9600\n");
      break;
}

  init_data_link(temp_time_out,temp_num_transmissions, baudrate);

}

int main(int argc, char **argv){

  if ((argc < 2) || ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
                     (strcmp("/dev/ttyS1", argv[1]) != 0))) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }

  printf("Hello ! Please choose the mode, Receiver(R)/Transmitter(T): ");
  int flagMode=0;
  char mode[1];

  do{

    //mode=getchar();

    scanf("%s",mode);

    if(strcmp(mode,"R")==0 || strcmp(mode,"T")==0){
      flagMode=1;
    }
    else{
      //fflush(stdout);
      printf("\nNot a possible mode, please select again: ");
    }

  }while(!flagMode);

  select_parameters();

  if(strcmp(mode,"R")==0){ //RECEIVER

    printf("\nRECEIVER.\n\n");

    int fd = set_up_connection(argv[1], RECEIVER);

    if (fd < 0) {
      printf("Error opening file descriptor. Exiting...\n");
      return -1;
    }

    receive_data();

    llclose(fd);

  } else if(strcmp(mode,"T")==0){ //TRANSMITTER

    printf("\nTRANSMITTER.\n\n");

    int fd = set_up_connection(argv[1], TRANSMITTER);

    if(fd < 0) {
      printf("Error opening file descriptor. Exiting...\n");
      return -1;
    }

    char path[] = ".";
    char filename[] = "pinguim.gif";
    if(send_data(path, filename) == -1)
      force_close(fd);
    else
      llclose(fd);


  }

  return 0;

}
