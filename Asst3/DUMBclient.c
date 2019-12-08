
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *masg);
void print_help_menu();
char *parse_read(char *user_command);



void error(const char *masg){

  perror(masg);
  exit(0);
}
char *parse_read(char *user_command){

  printf("user_command->   %s\n", user_command);
  int string_length = strlen(user_command);
  char *x = (char *)malloc(sizeof(char) * string_length);
  strncpy(x, user_command, string_length-1);



    if (strcmp("quit", x) == 0){
      printf("I am here - > GDBY");
      return "GDBY";
  }

  if (strcmp(x, "create") == 0){
    printf("I am here - > CREAT");
     return "CREAT";
  }

  if (strcmp(x, "delete") == 0){
    printf("I am here - > DELBX");
    return "DELBX";
  }

  if (strcmp(x, "open") == 0){
    printf("I am here - > OPNBX");
    return "OPNBX";
  }

  if (strcmp(x, "close") == 0){
    printf("I am here - > CLSBX");
    return "CLSBX";
  }

  if (strcmp(x, "next") == 0){
    printf("I am here - > NXTMG");
    return "NXTMG";
  }

  if (strcmp(x, "put") == 0){
    printf("I am here - > PUTMG");
    return "PUTMG";
  }

  if (strcmp(x, "help") == 0){
    printf("I am here - > HELP");
    //call function print manue..
    print_help_menu();
  }

  return "That is not a command, for a command list enter 'help'.";
}

void print_help_menu(){

  printf("\t----MAIN MENU-----\n");
  printf("\tPlease use one of the following commands:\n");
  printf("\t1. quit - stop session and exit the program\n");
  printf("\t2. create - Create new massage box on the server\n");
  printf("\t3. delete - will exit the program\n");
  printf("\t4. open - open if massage box if exists\n");
  printf("\t5. close - close current massage box\n");
  printf("\t6. next - Get the next massage from the current massage box\n");
  printf("\t7. put - Put a message in to the currently open message box\n");
  printf("\t8. help - will show main manue\n");
}

int main(int argc, char *argv[]) {

  int sockfd, portno, n, counter = 0;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char buffer[255];


  if(argc < 3){
    fprintf(stderr, "usage %s hostname port\n", argv[0]);
    exit(0);
  }

  portno = atoi(argv[2]);
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0){
    error("ERROR Opening socket");
  }

  server = gethostbyname(argv[1]);
  if(server == NULL){
    fprintf(stderr, "Error, no such host");
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *) server -> h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);
  //add trhee times try -> if no one takes the phone -> discconect and exit.
  if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
    error("Connection Failed");
  }
  printf("Server: connection successful\n");

  while(1){
    //clean buffer
    bzero(buffer, 255);
    char *fg = fgets(buffer, 255, stdin);
    char *send_command_to_server = parse_read(fg);
    printf("FG - strlen - > > %lu\n", strlen(fg));
    // printf("Command-> %s\n", send_command_to_server);
    //send the massage to the server..
    n = write(sockfd, send_command_to_server, strlen(send_command_to_server));

    if(n < 0){
      error("Error on writing");
    }
    //clean buffer.
    bzero(buffer, 255);
    //read massages from the server/ holds massages back from the server...
    n = read(sockfd, buffer, 255);
    if (n < 0){
      error("Error on reading");
    }


    printf("Server: %s\n", buffer);

    int i = strncmp("Bye", buffer, 3);
    if (i == 0){
      break;
    }

  }


  close(sockfd);
  return 0;

}
