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
#include <arpa/inet.h>
#include <time.h>

#define MAX_CONNECTIONS 16
#define MAX_MAILBOXES 8
#define MAILBOX_NAME_LEN_MAX    16
#define MAX_CMD_NAME_LEN 5

#define CMD_DELIMITER ' '

pthread_mutex_t global_lock;

void lock()
{
    pthread_mutex_lock(&global_lock);
}

void unlock()
{
    pthread_mutex_unlock(&global_lock);
}

void error(const char *masg)
{
    perror(masg);
    exit(1);
}


typedef struct mailbox
{
    char active_mailbox[MAILBOX_NAME_LEN_MAX];
    int active_clients_ref_counter;
} mailbox_t;

mailbox_t mailbox_table[MAX_MAILBOXES];

typedef struct client_context
{
    int socket_fd;
    pthread_t tid;
    struct sockaddr_in address;
    struct mailbox *activ_mailbox;
} client_context_t;


char *month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

int *time_string(char *buffer)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    return sprintf(buffer, "%02d %s ", tm.tm_mday, month[tm.tm_mon]);
}

char *message_prefix(char *buffer, client_context_t *client_ctx)
{
    int n = sprintf(buffer, "[%u] ", pthread_self());
    printf("%d\n", n);
    time_string(buffer + n);
    n += 7;
    printf("%d\n", n);
    struct sockaddr_in* pV4Addr = (struct sockaddr_in*) &client_ctx->address;
    struct in_addr ipAddr = pV4Addr->sin_addr;
    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ipAddr, str, INET_ADDRSTRLEN);
    n += sprintf((buffer + n), "%s", str);
    printf("%d\n", n);
    return buffer;
}

client_context_t clients_table[MAX_CONNECTIONS];

client_context_t *client_find_not_safe(pthread_t tid)
{
    int i = 0;
    for (; i < MAX_CONNECTIONS; i++)
    {
        if (tid == clients_table[i].tid)
        {
            return &clients_table[i];
        }
    }
    return NULL;
}

void client_remove_by_tid(pthread_t tid)
{
    lock();
    client_context_t * client = client_find_not_safe(tid);
    if (client)
    {
        client->tid = 0;
    }
    unlock();
}

void client_remove(client_context_t *client)
{
    client->tid = 0;
}

void client_shutdown(client_context_t *client)
{
    lock();
    do
    {
        if (!client->tid)
        {
            perror("ERROR: Client %p already removed!\n");
            break;
        }
        if (client->activ_mailbox)
        {
            client->activ_mailbox->active_clients_ref_counter--;
            if (client->activ_mailbox->active_clients_ref_counter == 0)
            {
                //TBD remove the mailbox - check the requirements
            }
        }
        close(client->socket_fd);
        client_remove(client);
    } while (0);
    unlock();
    char msg_buff[128];
    printf("%s disconnected\n", message_prefix(msg_buff, client));
}

client_context_t *client_add()
{
    client_context_t *client = NULL;
    lock();
    int i = 0;
    for (; i < MAX_CONNECTIONS; i++)
    {
        if (clients_table[i].tid == 0)
        {
            client = &clients_table[i];
            break;
        }
    }
    unlock();
    return client;
}

int parse_command(client_context_t *client_ctx, char *instruction, size_t len);

#define SOCKET_BUFF_SIZE 256

void* client_thread(void* arg)
{
    client_context_t *client_ctx = (client_context_t *) arg;

    char msg_buff[128];
    printf("%s connected\n", message_prefix(msg_buff, client_ctx));
    unsigned char buffer[SOCKET_BUFF_SIZE * 2];
    while (1)
    {
        bzero(buffer, SOCKET_BUFF_SIZE);
        int n = read(client_ctx->socket_fd, buffer, SOCKET_BUFF_SIZE);
        if (n == 0)
        {
            break;
        }
        if (n < 0)
        {
            error("Error on reading");
        }
        buffer[n - 1] = 0;

        //this will return a number for a specific case, then take it to the assigned action.
        int num_act = parse_command(client_ctx, buffer, n);

        //printf("Client : %s Action: %d\n", buffer, num_act);
        // process_action(num_act);
        //this need to be changed

        //        bzero(buffer, 255);
        //        fgets(buffer, 255, stdin);


        //        n = write(client_ctx->socket_fd, buffer, strlen(buffer));
        //
        //        if (n < 0)
        //        {
        //            error("Error on writing");
        //        }
        //
        //        int i = strncmp("Bye", buffer, 3);
        //        if (i == 0)
        //        {
        //            break;
        //        }
    }
    client_shutdown(client_ctx);
}

void on_end()
{
    int i = 0;
    for (; i < MAX_CONNECTIONS; i++)
    {
        if (clients_table[i].tid != 0)
        {
            printf("[%i]: Waiting on tid: [%lu]\n", i, clients_table[i].tid);
            pthread_join(clients_table[i].tid, NULL);
        }
    }
}

int main(int argc, char const *argv[])
{

    printf("DUMP Started\n");
    // check if port number is missing.
    if (argc < 2)
    {
        fprintf(stderr, "Port number not provided\n");
        exit(1);
    }

    memset(clients_table, 0, sizeof (clients_table));
    memset(mailbox_table, 0, sizeof (mailbox_table));

    int sockfd, newsockfd, portno, n;
    char buffer[255];

    struct sockaddr_in server_addr;


    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error("Error opening  Socket.");
    }

    bzero((char *) &server_addr, sizeof (server_addr));
    portno = atoi(argv[1]);

    printf("Listen on port: %u\n", portno);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof (server_addr)) < 0)
    {
        error("Binding Failed");
    }

    if (pthread_mutex_init(&global_lock, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
        return 1;
    }

    // open foe connections.
    listen(sockfd, MAX_CONNECTIONS);

    while (1)
    {
        client_context_t *cient_ctx = client_add();
        memset(cient_ctx, 0, sizeof (client_context_t));

        if (!cient_ctx)
        {
            fprintf(stderr, "No more clients can be connected\n");
            sleep(15);
        }

        socklen_t clilen = sizeof (cient_ctx->address);
        // "pick up the phone" and start interaction with user.
        cient_ctx->socket_fd = accept(sockfd, (struct sockaddr *) &cient_ctx->address, &clilen);

        if (cient_ctx->socket_fd < 0)
        {
            fprintf(stderr, "Error on accept! %u", cient_ctx->socket_fd);
            continue;
        }
        //        int n = read(cient_ctx.socket_fd, buffer, 5);
        //        
        //        printf("n %d %d\n", n, cient_ctx.socket_fd);

        int err = pthread_create(
            &cient_ctx->tid,
            NULL,
            &client_thread,
            cient_ctx);

        if (err != 0)
        {
            fprintf(stderr, "\nThread can't be created :[%s]", strerror(err));
            exit(1);
        }
    }


    close(sockfd);

    on_end();

    return 0;
}


int parse_command(client_context_t *client_ctx, char *instruction, size_t len)
{
    char msg_buff[128];
    //MAX char for command is 5+1.
    char command[MAX_CMD_NAME_LEN + 1] = "";
    //CMD_DELIMITER
    if (len < (MAX_CMD_NAME_LEN + 1))
    {
        fprintf(stderr, "%s [%d] Corrupted Command\n", message_prefix(msg_buff, client_ctx), __LINE__);
        return 0;
    }
    if (instruction[5] != CMD_DELIMITER)
    {
        fprintf(stderr, "%s [%d] Corrupted Command\n", message_prefix(msg_buff, client_ctx), __LINE__);
        return 0;
    }
    int i;
    for (i = 0; i < MAX_CMD_NAME_LEN; i++)
    {
        if ((isalpha(instruction[i]) == 0) || (isupper(instruction[i]) == 0))
        {
            fprintf(stderr, "%s [%d] Corrupted Command\n", message_prefix(msg_buff, client_ctx), __LINE__);
            return 0;
        }
        if (instruction[i] == ' ')
        {
            fprintf(stderr, "%s [%d] Corrupted Command\n", message_prefix(msg_buff, client_ctx), __LINE__);
            return 0;
        }
    }
    strncpy(command, instruction, MAX_CMD_NAME_LEN);

    int opcode = 0;
    //return num for each case: 1 - hello, 2 - goodbye!, 3 - create, 4 - open, 5 - next, 6 - put, 7 - delete, 8 - close.
    if (strcmp("HELLO", command) == 0)
    {
        opcode = 1;
    } else
        if (strcmp("GDBYE", command) == 0)
    {
        opcode = 2;
    } else
        if (strcmp("CREAT", command) == 0)
    {
        opcode = 3;
    } else
        if (strcmp("OPNBX", command) == 0)
    {
        opcode = 4;
    } else
        if (strcmp("NXTMG", command) == 0)
    {
        opcode = 5;
    } else
        if (strcmp("PUTMG", command) == 0)
    {
        opcode = 6;
    } else
        if (strcmp("DELBX", command) == 0)
    {
        opcode = 7;
    } else
        if (strcmp("CLSBX", command) == 0)
    {
        opcode = 8;
    } else
    {
        fprintf(stderr, "%s That is not a command, please try 'help' to discover more commands.", message_prefix(msg_buff, client_ctx));
        return 0;
    }

    printf("%s %s\n", message_prefix(msg_buff, client_ctx), command);
    return opcode;
}
