/*
filename server_ipaddress portno

argv[0] filename
argv[1] server_ipaddress
argv[2] portno
*/


/*
TASKS To DO:
1. add errors.
2. create multithreads for new massages/ boxes.
3. save massagases in queue.
4. operate the queue according to the user prompts.

*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

#define MAX_CONNECTIONS 8

void error(const char *masg){

  perror(masg);
  exit(1);
}

// void *work_procces(
//
//   return NULL;
// )

void parse_client_commands(char *client_command, int arguments){

  int x = strlen(client_command);
  char *str_command = (char *)malloc(sizeof(char) * x);
  strncpy(str_command, client_command, x-1);

  if (strcmp(str_command, "CREAT") == 0 || strcmp(str_command, "OPNBX") == 0 || strcmp(str_command, "PUTMG") == 0 ||
  strcmp(str_command, "DELBX") == 0 || strcmp(str_command, "CLSBX") == 0){

  }


}

int main(int argc, char const *argv[]) {

  if(argc < 2){
    fprintf(stderr, "Port number not provided\n");
    exit(1);
  }

  int sockfd, newsockfd, portno, n;
  char buffer[255];

  struct sockaddr_in server_addr, cli_addr;
  socklen_t clilen;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0){
    error("Error opening  Socket.");
  }

  bzero((char *) &server_addr, sizeof(server_addr));
  portno = atoi(argv[1]);

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(portno);

  if(bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
  error("Binding Failed");
  }

  listen(sockfd, MAX_CONNECTIONS);
  clilen = sizeof(cli_addr);

  newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

  if(newsockfd < 0){
    error("Error on accept!");
  }
  //create a new threat here
  pthread_t newthread;


  while(1){
    // crete a new thread and pass the chat function with the client.
    // pthread_create(&newthread, NULL, work_procces, NULL);

    bzero(buffer, 255);
    n = read(newsockfd, buffer, 255);
    if(n < 0){
        error("Error on reading");
      }

    //this need to be changed
    printf("Client : %s\n", buffer);
    bzero(buffer, 255);
    fgets(buffer, 255, stdin);


    n = write(newsockfd, buffer, strlen(buffer));

    if (n < 0){
      error("Error on writing");
    }

    int i = strncmp("Bye", buffer, 3);
    if (i == 0){
      break;
      }
    }

    close(newsockfd);
    close(sockfd);

   return 0;
}
