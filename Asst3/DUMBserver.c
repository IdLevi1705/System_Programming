/*
filename server_ipaddress portno

argv[0] filename
argv[1] server_ipaddress
argv[2] portno
*/


/*
TASKS To DO:
1. add errors.
2. create multi-threads for new massages/ boxes.
3. save messages in queue.
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
#include <ctype.h>

#define MAX_CONNECTIONS 16


void error(const char *masg){

  perror(masg);
  exit(1);
}

int parse_command(char *instruction){

  //MAX char for command is 5+1.
  char *command = (char *)malloc(sizeof(char) * 6);
  //start iterate from instruction string.
  int x = strlen(instruction);
  int i;
  for (i = 0; i < x ; i++){
    if ( (isalpha(instruction[i]) == 0) && (isupper(instruction[i]) == 0) )   {
      break;
    }
    if (instruction[i] == ' '){
      break;
    }
    command[i] = instruction[i];
  }
  //return num for each case: 1 - hello, 2 - goodbye!, 3 - create, 4 - open, 5 - next, 6 - put, 7 - delete, 8 - close.
  if (strcmp("HELLO", command) == 0){
    return 1;
  }
  if (strcmp("GDBY", command) == 0){
    return 2;
  }
  if (strcmp("CREAT", command) == 0){
    return 3;
  }
  if (strcmp("OPNBX", command) == 0){
    return 4;
  }
  if (strcmp("NXTMG", command) == 0){
    return 5;
  }
  if (strcmp("PUTMG", command) == 0){
    return 6;
  }
  if (strcmp("DELBX", command) == 0){
    return 7;
  }
  if (strcmp("CLSBX", command) == 0){
    return 8;
  }

  if (strcmp("FAIL", command) == 0){
    printf("That is not a command, please try 'help' to discover more commands.");
  }

  return 0;
}


int main(int argc, char const *argv[]) {

  // check if port number is missing.
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
  // open foe connections.
  listen(sockfd, MAX_CONNECTIONS);
  clilen = sizeof(cli_addr);
  // "pick up the phone" and start interaction with user.
  newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

  if(newsockfd < 0){
    error("Error on accept!");
  }
  //create a new threat here.
  // this should be give for every user and every connection since we want all user to interact with the serer at the same time instead of waiting on line.
  pthread_t newthread;


  while(1){
    // crete a new thread and pass the DUMB function with the client.
    // We need to create main function that work with the user - it will be our main function for every thread.
    // once that function ends we exit the program per thread.
    // pthread_create(&newthread, NULL, work_procces, NULL);

    bzero(buffer, 255);
    n = read(newsockfd, buffer, 255);
    if(n < 0){
        error("Error on reading");
      }
    //this will return a number for a specific case, then take it to the assigned action.
    // int num_act = parse_command(buffer);

    // process_action(num_act);
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
