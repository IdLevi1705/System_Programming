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

#define MAX_CONNECTIONS 1024
#define MAX_MAILBOXES 1024
#define MAILBOX_NAME_LEN_MIN    5
#define MAILBOX_NAME_LEN_MAX    25
#define MAX_CMD_NAME_LEN 5

#define CMD_DELIMITER ' '

void dump_array(char* buff, size_t len)
{

    struct temp
    {

        union
        {
            unsigned int i;
            char c[4];
        } u;
    };
    int i = 0;
    int counter = 0;
    int print_hexa = 1;
    for(; i < len; i++)
    {
        if(!(counter++ % 16))
        {
            if(1 == print_hexa && i != 0)
            {
                i -= 16;
                print_hexa = 0;
                printf("\t");
            }
            else
            {
                print_hexa = 1;
                printf("\n%p ", &buff[i]);
            }
        }
        if(print_hexa)
        {
            struct temp t = {0};
            t.u.c[0] = buff[i];
            printf("0x%02X ", t.u.i);
        }
        else
        {
            if(32 <= buff[i] && buff[i] <= 126)
            {
                printf("%c", buff[i]);
            }
            else
            {
                printf(".");
            }
        }
    }
    printf("\n");
}

#define DUMP_ARRAY(a,b) dump_array(a,b);

#define QUEUE_SIZE 1024
#define SIZE_MASK (QUEUE_SIZE - 1)

typedef enum
{
    QUEUE_STATUS_FULL = -1,
    QUEUE_STATUS_SUCCESS = 0
} queue_status_e;

typedef struct message
{
    unsigned char len;
    char msg[256];
} message_t;

typedef struct message_queue
{
    size_t headIndex_;
    size_t tailIndex_;
    message_t *circBuffer[QUEUE_SIZE];
} message_queue_t;

int msgq_is_empty(message_queue_t *msgq)
{
    // queue is considered empty if head is same as tail
    return msgq->headIndex_ == msgq->tailIndex_;
}

size_t msgq_next_index(size_t index)
{
    return(index + 1) & SIZE_MASK;
}

size_t msgq_prev_index(size_t index)
{
    printf("%u\n", __LINE__);
    size_t pindex = ((index - 1) & SIZE_MASK);
    printf("pindex: %d\n",  pindex);
}

int msgq_is_full(message_queue_t *msgq)
{
    printf("%u %p\n", __LINE__, msgq);
    printf("Tail %d, Head: %d\n", msgq->tailIndex_, msgq->headIndex_);
    return(msgq->tailIndex_ == msgq_prev_index(msgq->headIndex_));
}

void msgq_init(message_queue_t *msgq)
{
    memset(msgq, 0, sizeof(message_queue_t));
}

queue_status_e msgq_push_back(message_queue_t *msgq, message_t *msg)
{
    printf("%u %p\n", __LINE__, msgq);
    if(msgq_is_full(msgq))
    {
        return QUEUE_STATUS_FULL;
    }
    printf("%u\n", __LINE__);
    msgq->circBuffer[msgq->tailIndex_] = msg;
    msgq->tailIndex_ = msgq_next_index(msgq->tailIndex_);
    return QUEUE_STATUS_SUCCESS;
}

message_t * msgq_pup_front(message_queue_t *msgq)
{
    if(msgq_is_empty(msgq))
    {
        return(message_t *) 0;
    }
    message_t * node = msgq->circBuffer[msgq->headIndex_];
    msgq->headIndex_ = msgq_next_index(msgq->headIndex_);
    return node;
}

typedef enum
{
    STATUS_OK = 0,
    STATUS_CORRUPTED_CMD,
    STATUS_MBOX_OPENED,
    STATUS_MBOX_EXIST,
    STATUS_MBOX_NOT_EXIST
} MBOX_STATUS;

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
    int active;
    char mailbox_name[MAILBOX_NAME_LEN_MAX];
    void *client;
    message_queue_t msgq;
} mailbox_t;

mailbox_t mailbox_table[MAX_MAILBOXES];

mailbox_t *mailbox_find_by_name_not_safe(char *mbox_name)
{
    mailbox_t *mailbox = NULL;
    int i = 0;
    for(; i < MAX_MAILBOXES; i++)
    {
        if(0 == strncmp(mailbox_table[i].mailbox_name, mbox_name, MAILBOX_NAME_LEN_MAX))
        {
            mailbox = &mailbox_table[i];
            break;
        };
    }
    return mailbox;
}

MBOX_STATUS mailbox_open(char *mbox_name, void *client, mailbox_t **mbox)
{
    MBOX_STATUS status = STATUS_OK;
    int name_len = strlen(mbox_name);
    if(MAILBOX_NAME_LEN_MIN > name_len || name_len > MAILBOX_NAME_LEN_MAX)
    {
        // Wrong name length. Fail the request
        return STATUS_CORRUPTED_CMD;
    }
    lock();
    do
    {
        // Check if the mailbox is created already 
        mailbox_t *mailbox = mailbox_find_by_name_not_safe(mbox_name);
        if(mailbox == NULL)
        {
            status = STATUS_MBOX_NOT_EXIST;
            break;
        }
        if(mailbox->client == NULL) // No client opened it
        {
            mailbox->client = client;
            *mbox = mailbox;
            break;
        }
        if(mailbox->client != client) // someone opened it but not us
        {
            status = STATUS_MBOX_OPENED;
            break;
        }
        mailbox->client = client;
        *mbox = mailbox;
    } while(0);

    unlock();
    return status;
}

MBOX_STATUS mailbox_close(char *mbox_name, void *client)
{
    MBOX_STATUS status = STATUS_OK;
    int name_len = strlen(mbox_name);
    if(MAILBOX_NAME_LEN_MIN > name_len || name_len > MAILBOX_NAME_LEN_MAX)
    {
        // Wrong name length. Fail the request
        return STATUS_CORRUPTED_CMD;
    }
    lock();
    do
    {
        // Check if the mailbox is created already 
        mailbox_t *mailbox = mailbox_find_by_name_not_safe(mbox_name);
        if(mailbox == NULL)
        {
            status = STATUS_MBOX_NOT_EXIST;
            break;
        }
        if(mailbox->client != client)
        {
            status = STATUS_MBOX_OPENED;
            break;
        }
        mailbox->client = NULL;
    } while(0);
    unlock();
    return status;
}

MBOX_STATUS mailbox_delete(char *mbox_name)
{
    MBOX_STATUS status = STATUS_OK;
    int name_len = strlen(mbox_name);
    if(MAILBOX_NAME_LEN_MIN > name_len || name_len > MAILBOX_NAME_LEN_MAX)
    {
        // Wrong name length. Fail the request
        return STATUS_CORRUPTED_CMD;
    }
    lock();
    do
    {
        // Check if the mailbox is created already 
        mailbox_t *mailbox = mailbox_find_by_name_not_safe(mbox_name);
        if(mailbox == NULL)
        {
            status = STATUS_MBOX_NOT_EXIST;
            break;
        }
        if(mailbox->client)
        {
            status = STATUS_MBOX_OPENED;
            break;
        }
        mailbox->active = 0;
        mailbox->mailbox_name[0] = 0;
        status = STATUS_OK;
    } while(0);
    unlock();
    return status;
}

MBOX_STATUS mailbox_create(char *mbox_name, mailbox_t **mailbox)
{
    MBOX_STATUS status = STATUS_OK;
    *mailbox = NULL;
    int name_len = strlen(mbox_name);
    if(MAILBOX_NAME_LEN_MIN > name_len || name_len > MAILBOX_NAME_LEN_MAX)
    {
        // Wrong name length. Fail the request
        return STATUS_CORRUPTED_CMD;
    }
    lock();
    do
    {
        // Check if the mailbox is created already 
        *mailbox = mailbox_find_by_name_not_safe(mbox_name);
        if(*mailbox)
        {
            // MB exists. fail the request
            *mailbox = NULL;
            status = STATUS_MBOX_EXIST;
            break;
        }
        int i = 0;
        for(; i < MAX_MAILBOXES; i++)
        {
            if(0 == mailbox_table[i].active)
            {
                // empty slot found. let's use it
                mailbox_table[i].active = 1;
                strncpy(mailbox_table[i].mailbox_name, mbox_name, MAILBOX_NAME_LEN_MAX);
                *mailbox = &mailbox_table[i];
                status = STATUS_OK;
                break;
            };
        }
    } while(0);
    unlock();
    return status;
}

typedef struct client_context
{
    int socket_fd;
    pthread_t tid;
    struct sockaddr_in address;
    struct mailbox *open_mailbox;

} client_context_t;

char *month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

int time_string(char *buffer)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    return sprintf(buffer, "%02d %s ", tm.tm_mday, month[tm.tm_mon]);
}

char *skip_leading_spaces(char *string)
{
    if(*string == 0)
    {
        return string;
    }
    while(*string == ' ')
    {
        string++;
    }
    return string;
}

char *message_prefix(char *buffer, client_context_t * client_ctx)
{
    int n = sprintf(buffer, "[%u] ", (unsigned int) pthread_self());
    n += time_string(buffer + n);
    struct sockaddr_in* pV4Addr = (struct sockaddr_in*) &client_ctx->address;
    struct in_addr ipAddr = pV4Addr->sin_addr;
    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ipAddr, str, INET_ADDRSTRLEN);
    n += sprintf((buffer + n), "%s", str);
    buffer[n] = 0;
    return buffer;
}

client_context_t clients_table[MAX_CONNECTIONS];

client_context_t * client_find_not_safe(pthread_t tid)
{
    int i = 0;
    for(; i < MAX_CONNECTIONS; i++)
    {
        if(tid == clients_table[i].tid)
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
    if(client)
    {
        client->tid = 0;
    }
    unlock();
}

void client_remove(client_context_t * client)
{
    client->tid = 0;
}

void client_shutdown(client_context_t * client)
{
    lock();
    do
    {
        if(!client->tid)
        {
            perror("ERROR: Client %p already removed!\n");
            break;
        }
        if(client->open_mailbox)
        {
            client->open_mailbox->client = NULL;
        }
        shutdown(client->socket_fd, SHUT_RDWR);
        close(client->socket_fd);
        client_remove(client);
    } while(0);
    unlock();
    char msg_buff[128];
    printf("%s disconnected\n", message_prefix(msg_buff, client));
}

client_context_t * client_add()
{
    client_context_t *client = NULL;
    lock();
    int i = 0;
    for(; i < MAX_CONNECTIONS; i++)
    {
        if(clients_table[i].tid == 0)
        {
            client = &clients_table[i];
            break;
        }
    }
    unlock();
    return client;
}

int send_to_client(client_context_t *clisnt_ctx, char *buffer);
int read_from_client(client_context_t *clisnt_ctx, char *buffer);

int parse_command(client_context_t *client_ctx, char *instruction, size_t len);
int parse_args(client_context_t *client_ctx, int opcode, char *instruction, size_t len);

#define SOCKET_BUFF_SIZE 256

void* client_thread(void* arg)
{
    client_context_t *client_ctx = (client_context_t *) arg;

    char msg_buff[128];
    printf("%s connected\n", message_prefix(msg_buff, client_ctx));
    char buffer[SOCKET_BUFF_SIZE + 1];
    while(1)
    {
        bzero(buffer, SOCKET_BUFF_SIZE);
        int n = read(client_ctx->socket_fd, buffer, SOCKET_BUFF_SIZE);
        if(n == 0)
        {
            break;
        }
        if(n < 0)
        {
            error("Error on reading");
        }
        buffer[n] = 0;

        //this will return a number for a specific case, then take it to the assigned action.
        int num_act = parse_command(client_ctx, buffer, n);
    }
    client_shutdown(client_ctx);
}

void on_end()
{
    int i = 0;
    for(; i < MAX_CONNECTIONS; i++)
    {
        if(clients_table[i].tid != 0)
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
    if(argc < 2)
    {
        fprintf(stderr, "Port number not provided\n");
        exit(1);
    }
    memset(clients_table, 0, sizeof(clients_table));
    memset(mailbox_table, 0, sizeof(mailbox_table));

    int i = 0;
    for(; i < MAX_MAILBOXES; i++)
    {
        msgq_init(&mailbox_table[i].msgq);
    }

    int sockfd, portno;
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        error("Error opening  Socket.");
    }

    bzero((char *) &server_addr, sizeof(server_addr));
    portno = atoi(argv[1]);

    printf("Listen on port: %u\n", portno);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(portno);

    if(bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        error("Binding Failed");
    }

    if(pthread_mutex_init(&global_lock, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
        return 1;
    }

    // open foe connections.
    listen(sockfd, MAX_CONNECTIONS);

    while(1)
    {
        client_context_t *cient_ctx = client_add();
        memset(cient_ctx, 0, sizeof(client_context_t));

        if(!cient_ctx)
        {
            fprintf(stderr, "No more clients can be connected\n");
            sleep(15);
        }

        socklen_t clilen = sizeof(cient_ctx->address);
        // "pick up the phone" and start interaction with user.
        cient_ctx->socket_fd = accept(sockfd, (struct sockaddr *) &cient_ctx->address, &clilen);

        if(cient_ctx->socket_fd < 0)
        {
            fprintf(stderr, "Error on accept! %u", cient_ctx->socket_fd);
            continue;
        }
        int err = pthread_create(
                                 &cient_ctx->tid,
                                 NULL,
                                 &client_thread,
                                 cient_ctx);

        if(err != 0)
        {
            fprintf(stderr, "\nThread can't be created :[%s]", strerror(err));
            exit(1);
        }
    }

    close(sockfd);

    on_end();

    return 0;
}

void HELLO_handler(client_context_t *client_ctx, char *instruction);
void GDBYE_handler(client_context_t *client_ctx, char *instruction);
void CREAT_handler(client_context_t *client_ctx, char *instruction);
void OPNBX_handler(client_context_t *client_ctx, char *instruction);
void NXTMG_handler(client_context_t *client_ctx, char *instruction);
void PUTMG_handler(client_context_t *client_ctx, char *instruction);
void DELBX_handler(client_context_t *client_ctx, char *instruction);
void CLSBX_handler(client_context_t *client_ctx, char *instruction);
void Error_handler(client_context_t *client_ctx, char *instruction);

int respond_to_client(client_context_t *client_ctx, char *msg)
{
    char msg_buff[128];
    printf("%s %s \n", message_prefix(msg_buff, client_ctx), msg);
    return send_to_client(client_ctx, msg);
}

int respond_WHAT(client_context_t *client_ctx)
{
    return respond_to_client(client_ctx, "ER:WHAT?");
}

int respond_OK(client_context_t *client_ctx)
{
    return respond_to_client(client_ctx, "OK!");
}

int parse_command(client_context_t *client_ctx, char *instruction, size_t len)
{
    DUMP_ARRAY(instruction, 33);
    char msg_buff[256];
    //MAX char for command is 5+1.
    char command[MAX_CMD_NAME_LEN + 1] = "";
    //CMD_DELIMITER
    if(len < (MAX_CMD_NAME_LEN))
    {
        fprintf(stderr, "%s [%d] Corrupted Command\n", message_prefix(msg_buff, client_ctx), __LINE__);
        return 0;
    }
    if(instruction[5] != CMD_DELIMITER && instruction[5] != 0)
    {
        fprintf(stderr, "%s [%d] Corrupted Command\n", message_prefix(msg_buff, client_ctx), __LINE__);
        return 0;
    }
    int i;
    for(i = 0; i < MAX_CMD_NAME_LEN; i++)
    {
        if((isalpha(instruction[i]) == 0) || (isupper(instruction[i]) == 0))
        {
            fprintf(stderr, "%s [%d] Corrupted Command\n", message_prefix(msg_buff, client_ctx), __LINE__);
            return 0;
        }
        if(instruction[i] == ' ')
        {
            fprintf(stderr, "%s [%d] Corrupted Command\n", message_prefix(msg_buff, client_ctx), __LINE__);
            return 0;
        }
    }
    instruction[MAX_CMD_NAME_LEN] = 0;
    strncpy(command, instruction, MAX_CMD_NAME_LEN);
    instruction += (MAX_CMD_NAME_LEN + 1);

    DUMP_ARRAY(command, 17);
    DUMP_ARRAY(instruction, 17);

    int opcode = 0;
    //return num for each case: 1 - hello, 2 - goodbye!, 3 - create, 4 - open, 5 - next, 6 - put, 7 - delete, 8 - close.
    if(strcmp("HELLO", command) == 0)
    {
        opcode = 1;
        HELLO_handler(client_ctx, instruction);
    }
    else if(strcmp("GDBYE", command) == 0)
    {
        opcode = 2;
        GDBYE_handler(client_ctx, instruction);
    }
    else if(strcmp("CREAT", command) == 0)
    {
        opcode = 3;
        CREAT_handler(client_ctx, instruction);
    }
    else if(strcmp("OPNBX", command) == 0)
    {
        opcode = 4;
        OPNBX_handler(client_ctx, instruction);
    }
    else if(strcmp("NXTMG", command) == 0)
    {
        opcode = 5;
        NXTMG_handler(client_ctx, instruction);
    }
    else if(strcmp("PUTMG", command) == 0)
    {
        opcode = 6;
        PUTMG_handler(client_ctx, instruction);
    }
    else if(strcmp("DELBX", command) == 0)
    {
        opcode = 7;
        DELBX_handler(client_ctx, instruction);
    }
    else if(strcmp("CLSBX", command) == 0)
    {
        opcode = 8;
        CLSBX_handler(client_ctx, instruction);
    }
    else
    {
        fprintf(stderr, "%s That is not a command, please try 'help' to discover more commands.", message_prefix(msg_buff, client_ctx));
        Error_handler(client_ctx, instruction);
        return 0;
    }
    return opcode;
}

void HELLO_handler(client_context_t *client_ctx, char *instruction)
{
    char msg_buff[128];
    char buffer[SOCKET_BUFF_SIZE + 1];
    bzero(buffer, 255);
    sprintf(buffer, "HELLO DUMBv0 ready!\n");
    int n = write(client_ctx->socket_fd, buffer, strlen(buffer));
    if(n < 0)
    {
        fprintf(stderr, "%s Error on writing %u\n", message_prefix(msg_buff, client_ctx), n);
    }
    printf("%s %s %s\n", message_prefix(msg_buff, client_ctx), "HELLO", instruction);
}

void GDBYE_handler(client_context_t *client_ctx, char *instruction)
{
    char msg_buff[128];
    printf("%s %s %s\n", message_prefix(msg_buff, client_ctx), "GDBYE", instruction);
    client_shutdown(client_ctx);
    pthread_exit(NULL);
}

char *clean_args(char *args)
{
    char *str = skip_leading_spaces(args);
    char *ending_space = strchr(str, ' ');
    if(ending_space)
        *ending_space = 0;
    return str;
}

void CREAT_handler(client_context_t *client_ctx, char *instruction)
{
    DUMP_ARRAY(instruction, 17);
    char msg_buff[128];
    printf("%s %s %s\n", message_prefix(msg_buff, client_ctx), "CREAT", instruction);
    // get mbox name
    char *mbox_name = clean_args(instruction);

    mailbox_t *mbox = NULL;
    MBOX_STATUS status = mailbox_create(mbox_name, &mbox);
    switch(status)
    {
        case STATUS_OK:
            respond_OK(client_ctx);
            break;
        case STATUS_CORRUPTED_CMD:
            respond_WHAT(client_ctx);
            break;
        case STATUS_MBOX_EXIST:
            respond_to_client(client_ctx, "ER:EXIST");
            break;
        default:
            exit(0);
    }
}

void DELBX_handler(client_context_t *client_ctx, char *instruction)
{
    DUMP_ARRAY(instruction, 17);
    char msg_buff[128];
    printf("%s %s %s\n", message_prefix(msg_buff, client_ctx), "DELBX", instruction);
    // get mbox name
    char *mbox_name = clean_args(instruction);

    MBOX_STATUS status = mailbox_delete(mbox_name);
    switch(status)
    {
        case STATUS_OK:
            respond_OK(client_ctx);
            break;
        case STATUS_CORRUPTED_CMD:
            respond_WHAT(client_ctx);
            break;
        case STATUS_MBOX_OPENED:
            respond_to_client(client_ctx, "ER:OPEND");
            break;
        case STATUS_MBOX_NOT_EXIST:
            respond_to_client(client_ctx, "ER:NEXST");
            break;
        default:
            exit(0);
    }
}

void OPNBX_handler(client_context_t *client_ctx, char *instruction)
{
    DUMP_ARRAY(instruction, 17);
    char msg_buff[128];
    printf("%s %s %s\n", message_prefix(msg_buff, client_ctx), "OPNBX", instruction);
    // get mbox name
    char *mbox_name = clean_args(instruction);

    mailbox_t *mbox = NULL;
    MBOX_STATUS status = mailbox_open(mbox_name, client_ctx, &mbox);
    switch(status)
    {
        case STATUS_OK:
            // close previously opened MB
            if(client_ctx->open_mailbox)
            {
                client_ctx->open_mailbox->client = NULL;
            }
            client_ctx->open_mailbox = mbox;
            respond_OK(client_ctx);
            break;
        case STATUS_CORRUPTED_CMD:
            respond_WHAT(client_ctx);
            break;
        case STATUS_MBOX_OPENED:
            respond_to_client(client_ctx, "ER:OPEND");
            break;
        case STATUS_MBOX_NOT_EXIST:
            respond_to_client(client_ctx, "ER:NEXST");
            break;
        default:
            exit(0);
    }
}

void NXTMG_handler(client_context_t *client_ctx, char *instruction)
{
    DUMP_ARRAY(instruction, 17);
    char msg_buff[128];
    printf("%s %s %s\n", message_prefix(msg_buff, client_ctx), "NXTMG", instruction);

}

int mailbox_put_msg(mailbox_t *mbox, message_t *msg)
{
    printf("%u\n", __LINE__);
    if(QUEUE_STATUS_SUCCESS != msgq_push_back(&mbox->msgq, msg))
    {
        return 0;
    }
    return 1;
}

int check_if_ascii(char *string)
{
    while(*string)
    {
        if(32 > *string || *string > 126)
        {
            return 0;
        }
        string++;
    }
    return 1;
}

void PUTMG_handler(client_context_t *client_ctx, char *instruction)
{
    DUMP_ARRAY(instruction, 17);
    char msg_buff[128];
    printf("%s %s %s\n", message_prefix(msg_buff, client_ctx), "PUTMG", instruction);
    char *str = skip_leading_spaces(instruction);
    // check message format
    if(*str != '!')
    {
        respond_WHAT(client_ctx);
        return;
    }
    str++;
    printf("%u\n", __LINE__);
    char *start_msg = strchr(str, '!');
    if(!start_msg)
    {
        respond_WHAT(client_ctx);
        return;
    }
    printf("%u\n", __LINE__);
    *start_msg = 0;
    start_msg++;

    DUMP_ARRAY(str, 17);
    unsigned int expected_msg_len = atoi(str);
    unsigned int actual_msg_len = strlen(start_msg);
    printf("%u\n", __LINE__);

    if(expected_msg_len > 255)
    {
        printf("Error: The message is too long.  %d (Max = 255)\n", expected_msg_len);
        respond_WHAT(client_ctx);
        return;
    }
    printf("%u\n", __LINE__);
    if(expected_msg_len != actual_msg_len)
    {
        printf("Error: incorrect length of the message: Expected: %d. Actual: %d\n", expected_msg_len, actual_msg_len);
        respond_WHAT(client_ctx);
        return;
    }
    printf("%u\n", __LINE__);
    if(0 == check_if_ascii(start_msg))
    {
        printf("Error: The message supposed to be in ASCII alphanumeric format (32 - 126)\n");
        respond_WHAT(client_ctx);
        return;
    }
    printf("%u\n", __LINE__);
    if(!client_ctx->open_mailbox)
    {
        respond_to_client(client_ctx, "ER:NOOPN");
        return;
    }

    message_t *msg = (message_t *) malloc(sizeof(message_t));
    if(!msg)
    {
        perror("FATAL Error: memory allocation failed. Stop process");
        exit(1);
    }
    printf("%u\n", __LINE__);
    msg->len = expected_msg_len;
    strcpy(msg->msg, start_msg);
    printf("%u\n", __LINE__);
    if(!mailbox_put_msg(client_ctx->open_mailbox, msg))
    {
        free(msg);
        printf("Error: Failed to add put message to mailbox %s.\n", client_ctx->open_mailbox->mailbox_name);
        respond_to_client(client_ctx, "ER:NOOPN");
        return;
    }
    printf("%u\n", __LINE__);
    sprintf(msg_buff, "OK!%d",expected_msg_len); 
    respond_to_client(client_ctx, msg_buff);
}

void CLSBX_handler(client_context_t *client_ctx, char *instruction)
{
    DUMP_ARRAY(instruction, 17);
    char msg_buff[128];
    printf("%s %s %s\n", message_prefix(msg_buff, client_ctx), "CLSBX", instruction);
    // get mbox name
    char *mbox_name = clean_args(instruction);
    MBOX_STATUS status = mailbox_close(mbox_name, client_ctx);
    switch(status)
    {
        case STATUS_OK:
            client_ctx->open_mailbox = NULL;
            respond_OK(client_ctx);
            break;
        case STATUS_CORRUPTED_CMD:
            respond_WHAT(client_ctx);
            break;
        case STATUS_MBOX_OPENED:
            respond_to_client(client_ctx, "ER:NOOPN");
            break;
        case STATUS_MBOX_NOT_EXIST:
            respond_to_client(client_ctx, "ER:NOOPN");
            break;
        default:
            exit(0);
    }
}

void Error_handler(client_context_t *client_ctx, char *instruction)
{
    char msg_buff[128];
    char buffer[32] = "ER:WHAT?";
    printf("%s %s \n", message_prefix(msg_buff, client_ctx), buffer);
    send_to_client(client_ctx, buffer);
}

int send_to_client(client_context_t *client_ctx, char *buffer)
{
    int n = write(client_ctx->socket_fd, buffer, strlen(buffer));
    //printf("%i bytes sent to server\n", n);
    if(n < 0)
    {
        error("Error on reading");
    }
    return n;
}

int read_from_client(client_context_t *client_ctx, char *buffer)
{
    //read massages from the server/ holds massages back from the server...
    int n = read(client_ctx->socket_fd, buffer, 255);
    if(n < 0)
    {
        error("Error on reading");
    }
    if(n == 0)
    {
        printf("Socket can not be read from and was closed on server side\n");
        exit(0);
    }

    //printf("%i %s\n", n, buffer);
    return n;
}

