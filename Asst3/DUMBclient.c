
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>

void error(const char *masg);
void print_help_menu();
char *parse_read(char *user_command);
void parse_client_commands(char *client_command);


void error(const char *masg){

  perror(masg);
  exit(0);
}

// void parse_client_commands(char *client_command){
//   printf("Entered parse command function...");
//   int x = strlen(client_command);
//   char *str_command = (char *)malloc(sizeof(char) * x);
//   strncpy(str_command, client_command, x-1);
//
//
// }

char *parse_read(char *user_command){

  //MAX char for command is 6+1.
  char *command = (char *)malloc(sizeof(char) * 7);
  //start iterate from instruction string.
  int x = strlen(user_command);
  int i;
  for (i = 0; i < x ; i++){
    if ( (isalpha(user_command[i]) == 0) && (isupper(user_command[i]) == 0) )   {
      break;
    }
    if (user_command[i] == ' '){
      break;
    }
    command[i] = user_command[i];
  }


  if (strcmp("hello", command) == 0){
    return "HELLO";
  }

    if (strcmp("quit", command) == 0){
      return "GDBY";
  }

  if (strcmp("create", command) == 0){
     return "CREAT";
  }

  if (strcmp("delete", command) == 0){
    return "DELBX";
  }

  if (strcmp("open", command) == 0){
    printf("open, which message box?\n");
    return "OPNBX";
  }

  if (strcmp("close", command) == 0){
    return "CLSBX";
  }

  if (strcmp("next", command) == 0){
    return "NXTMG";
  }

  if (strcmp("put", command) == 0){
    return "PUTMG";
  }

  if (strcmp("help", command) == 0){
    printf("I am here - > HELP");
    //call function print manue..
    print_help_menu();
    return "HELP";
  }
  return "FAIL";
}

void print_help_menu(){

  printf("\t----MAIN MENU-----\n");
  printf("\tPlease use one of the following commands:\n");
  printf("\t0. hello - To start interaction with the server\n");
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
  char argument[20];


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
  } else {
    printf("\033[0;32m");
    printf("Server: connection successful\n");
    printf("\033[0m");
  }

  bzero(buffer, 255);
  char *fg = fgets(buffer, 255, stdin);
  char *send_command_to_server = parse_read(fg);
  if (strcmp("HELLO", send_command_to_server) != 0){
    printf("Fail to connect initilize the server, please try again..");
    return 0;
  }

  //talk to the server untill user types 'close'
  while(1){
    //clean buffer
    bzero(buffer, 255);
    char *fg = fgets(buffer, 255, stdin);
    char *send_command_to_server = parse_read(fg);


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
