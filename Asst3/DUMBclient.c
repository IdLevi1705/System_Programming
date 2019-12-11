
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>

#define USER_CMD_LEN_MAX 6
#define USER_CMD_LEN_MIN 3
#define ARGS_LEN_MAX 45
#define DUMB_CMD_LEN 6

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
    for (; i < len; i++)
    {
        if (!(counter++ % 16))
        {
            if (1 == print_hexa && i != 0)
            {
                i -= 16;
                print_hexa = 0;
                printf("\t");
            } else
            {
                print_hexa = 1;
                printf("\n");
            }
        }
        if (print_hexa)
        {
            struct temp t = {0};
            t.u.c[0] = buff[i];
            printf("0x%02X ", t.u.i);
        } else
        {
            if (32 <= buff[i] && buff[i] <= 126)
            {
                printf("%c", buff[i]);
            } else
            {
                printf(".");
            }
        }
    }
    printf("\n");
}

#define DUMP_ARRAY(a,b) //dump_array(a,b);

typedef enum
{
    USR_CMD_STATUS_OK = 0,
    USR_CMD_STATUS_BAD_CMD,
    USR_CMD_STATUS_BAD_ARGS
} usr_cmd_parse_status;

typedef enum
{
    DUMB_CMD_HELLO = 1,
    DUMB_CMD_GDBYE,
    DUMB_CMD_CREAT,
    DUMB_CMD_DELBX,
    DUMB_CMD_OPNBX,
    DUMB_CMD_CLSBX,
    DUMB_CMD_NXTMG,
    DUMB_CMD_PUTMG
} dumb_cmd_opcode;

typedef struct dumb_command
{
    dumb_cmd_opcode opcode;
    char user_command[USER_CMD_LEN_MAX + 5];
    char command[USER_CMD_LEN_MAX];
    char args[ARGS_LEN_MAX];
} dumb_command_t;


void error(const char *masg);
void print_help_menu();
usr_cmd_parse_status parse_read(char *user_command, dumb_command_t *command);
void parse_client_commands(char *client_command);

void error(const char *masg)
{
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

char *skip_leading_spaces(char *string)
{
    if (*string == 0)
    {
        return string;
    }
    while (*string == ' ')
    {
        string++;
    }
    return string;
}

#define ERR_PRINT(msg) printf("[%u] Error: %s\n", __LINE__, msg);

usr_cmd_parse_status parse_read(char *user_command, dumb_command_t *command)
{
    DUMP_ARRAY(user_command, 17);
    //start iterate from instruction string.
    command->user_command[0] = 0;
    command->args[0] = 0;
    int cmd_len = strlen(user_command);
    //printf("cmd_len: %d\n", cmd_len);
    if (cmd_len > (USER_CMD_LEN_MAX + ARGS_LEN_MAX) || cmd_len < USER_CMD_LEN_MIN)
    {
        ERR_PRINT("Bad Command");
        return USR_CMD_STATUS_BAD_CMD;
    }
    //printf("%u\n", __LINE__);
    char *cmd_args = NULL;
    //tokenize
    cmd_args = strchr(user_command, '\n');
    if (cmd_args)
    {
        *cmd_args = '\0';
        //printf("%u\n", __LINE__);
    }
    cmd_args = strchr(user_command, ' ');
    if (cmd_args)
    {
        *cmd_args = '\0';
        cmd_args++;
        cmd_args = skip_leading_spaces(cmd_args);
        //printf("%u\n", __LINE__);
    }

    //printf("%u\n", __LINE__);
    // check until reach end of the cmd string
    // we put \0 during initial parsing between command and args
    int i = 0;
    cmd_len = strlen(user_command);
    //printf("cmd_len: %d\n", cmd_len);
    for (i = 0; i < cmd_len; i++)
    {
        if ((isalpha(user_command[i]) == 0) && (isupper(user_command[i]) == 0))
        {
            ERR_PRINT("Bad Command");
            return USR_CMD_STATUS_BAD_CMD;
        }
    }
    //printf("%u\n", __LINE__);

    if (strcmp("quit", user_command) == 0)
    {
        command->opcode = DUMB_CMD_GDBYE;
        strcpy(command->user_command, "quit");
        strcpy(command->command, "GDBYE");
    } else if (strcmp("create", user_command) == 0)
    {
        command->opcode = DUMB_CMD_CREAT;
        DUMP_ARRAY(command, 32);
        strcpy(command->command, "CREAT");
        if (cmd_args)
            strncpy(command->args, cmd_args, ARGS_LEN_MAX);
    } else if (strcmp("delete", user_command) == 0)
    {
        command->opcode = DUMB_CMD_DELBX;
        DUMP_ARRAY(command, 32);
        strcpy(command->command, "DELBX");
        if (cmd_args)
            strncpy(command->args, cmd_args, ARGS_LEN_MAX);
    } else if (strcmp("open", user_command) == 0)
    {
        command->opcode = DUMB_CMD_OPNBX;
        DUMP_ARRAY(command, 32);
        strcpy(command->command, "OPNBX");
        if (cmd_args)
            strncpy(command->args, cmd_args, ARGS_LEN_MAX);
    } else if (strcmp("close", user_command) == 0)
    {
        command->opcode = DUMB_CMD_CLSBX;
        DUMP_ARRAY(command, 32);
        strcpy(command->command, "CLSBX");
        if (cmd_args)
            strncpy(command->args, cmd_args, ARGS_LEN_MAX);
    } else if (strcmp("next", user_command) == 0)
    {
        //return DUMP_CMD_DELBX_NXTMG";
    } else if (strcmp("put", user_command) == 0)
    {
        //return "PUTMG";
    } else if (strcmp("help", user_command) == 0)
    {
        printf("I am here - > HELP");
        print_help_menu();
    } else
    {
        ERR_PRINT("Bad Command");
        return USR_CMD_STATUS_BAD_CMD;
    }
    return USR_CMD_STATUS_OK;
}

void print_help_menu()
{

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

int send_cmd_to_server(dumb_command_t *command, int sockfd);
int read_from_server(dumb_command_t *command, int sockfd, char *buffer);

int main(int argc, char *argv[])
{

    int sockfd, portno, n, counter = 0, con = 0;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[255];
    char argument[20];

    if (argc < 3)
    {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error("ERROR Opening socket");
    }

    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "Error, no such host");
    }

    bzero((char *) &serv_addr, sizeof (serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *) server -> h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    char *hello = "HELLO";
    int i = 0;
    for (; i < 2; i++)
    {
        con = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr));
        if (con == 0)
        {
            break;
        }
        fprintf(stderr, "Failed to connect to DUMP server\n");
        sleep(1);
    }
    if (con)
    {
        error("Failed to connect. Exit Now\n");
    }

    dumb_command_t command = {0};
    command.opcode = DUMB_CMD_HELLO;
    send_to_server(&command, sockfd, hello);
    bzero(buffer, 255);
    read_from_server(&command, sockfd, buffer);
    printf("%s\n", buffer);

    //talk to the server until user types 'close'
    while (1)
    {
        //clean buffer
        printf("%s:> ", command.user_command);
        bzero(buffer, 255);
        char *user_cmd = fgets(buffer, 255, stdin);
        if (*user_cmd == '\n')
        {
            continue;
        }
        usr_cmd_parse_status status = parse_read(user_cmd, &command);

        //printf("status %u opcode: %u\n", status, command.opcode);

        if (USR_CMD_STATUS_BAD_CMD == status)
        {
            continue;
        } else if (USR_CMD_STATUS_BAD_ARGS == status)
        {
            continue;
        } else if (USR_CMD_STATUS_OK == status)
        {
            switch (command.opcode)
            {
            case DUMB_CMD_HELLO:
                printf("Hello State\n");
                break;
            case DUMB_CMD_GDBYE:
            {
                send_cmd_to_server(&command, sockfd);
                bzero(buffer, 255);
                if (read_from_server(&command, sockfd, buffer) > 0)
                {
                    printf("ERROR: Unexpected Server behavior. Closing....\n");
                    exit(1);
                }
            }
                break;
            case DUMB_CMD_CREAT:
            case DUMB_CMD_DELBX:
            case DUMB_CMD_OPNBX:
            case DUMB_CMD_CLSBX:
            {
                send_cmd_to_server(&command, sockfd);
                bzero(buffer, 255);
                read_from_server(&command, sockfd, buffer);
                printf("%s\n", buffer);
            }
                break;
            case DUMB_CMD_NXTMG:
                break;
            case DUMB_CMD_PUTMG:
                break;
            }

            //printf("Server resp: %s\n", buffer);
            continue;
        } else
        {
            continue;
        }

        //n = write(sockfd, send_command_to_server, strlen(send_command_to_server));

        if (n < 0)
        {
            error("Error on writing");
        }
        //clean buffer.
        bzero(buffer, 255);
        //read massages from the server/ holds massages back from the server...
        n = read(sockfd, buffer, 255);
        if (n < 0)
        {
            error("Error on reading");
        }


        printf("Server: %s\n", buffer);

        int i = strncmp("Bye", buffer, 3);
        if (i == 0)
        {
            break;
        }

    }

    close(sockfd);
    return 0;
}

int send_to_server(dumb_command_t *command, int sockfd, char *buffer)
{
    int n = write(sockfd, buffer, strlen(buffer));
    //printf("%i bytes sent to server\n", n);
    if (n < 0)
    {
        error("Error on reading");
    }
    return n;
}

int send_cmd_to_server(dumb_command_t *command, int sockfd)
{
    char cmd_buffer[128] = {0};
    int n = sprintf(cmd_buffer, "%s", command->command);
    if (command->args[0] != 0)
        n += sprintf(cmd_buffer + n, " %s", command->args);
    return send_to_server(command, sockfd, cmd_buffer);
}

int read_from_server(dumb_command_t *command, int sockfd, char *buffer)
{
    //read massages from the server/ holds massages back from the server...
    int n = read(sockfd, buffer, 255);
    if (n < 0)
    {
        error("Error on reading");
    }
    if (n == 0)
    {
        printf("Socket can not be read from and was closed on server side\n");
        exit(0);
    }

    //printf("%i %s\n", n, buffer);
    return n;
}
