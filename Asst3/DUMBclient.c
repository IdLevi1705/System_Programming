
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
#define ARGS_LEN_MAX 265
#define DUMB_CMD_LEN 6
#define READ_BUFFER_LEN 300

#define DEBUG_PRINT //printf
#define DUMP_ARRAY(a,b) //dump_array(a,b);

void
dump_array(char* buff, size_t len)
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
            }
            else
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
        }
        else
        {
            if (32 <= buff[i] && buff[i] <= 126)
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

dumb_cmd_opcode current_mode = DUMB_CMD_HELLO;
char open_mailbox[25];

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

void
error(const char *masg)
{
    perror(masg);
    exit(0);
}

char *
skip_leading_spaces(char *string)
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

int
check_if_ascii(char *string)
{
    while (*string)
    {
        if (32 > *string || *string > 126)
        {
            return 0;
        }
        string++;
    }
    return 1;
}

usr_cmd_parse_status
parse_read(char *user_command, dumb_command_t *command)
{
    DUMP_ARRAY(user_command, 17);
    //start iterate from instruction string.
    command->user_command[0] = 0;
    command->args[0] = 0;
    char *cmd_args = NULL;
    cmd_args = strchr(user_command, '\n');
    if (cmd_args)
    {
        *cmd_args = '\0';
    }
    cmd_args = strchr(user_command, ' ');
    if (cmd_args)
    {
        *cmd_args = '\0';
    }
    int cmd_len = strlen(user_command);
    if (cmd_len > (USER_CMD_LEN_MAX + ARGS_LEN_MAX) || cmd_len < USER_CMD_LEN_MIN)
    {
        ERR_PRINT("That is not a command, for a command list enter 'help'.");
        return USR_CMD_STATUS_BAD_CMD;
    }
    int i = 0;
    cmd_len = strlen(user_command);
    for (i = 0; i < cmd_len; i++)
    {
        if ((isalpha(user_command[i]) == 0) && (isupper(user_command[i]) == 0))
        {
            ERR_PRINT("That is not a command, for a command list enter 'help'.");
            return USR_CMD_STATUS_BAD_CMD;
        }
    }
    if (strcmp("quit", user_command) == 0)
    {
        command->opcode = DUMB_CMD_GDBYE;
        strcpy(command->user_command, "quit");
        strcpy(command->command, "GDBYE");
    }
    else if (strcmp("create", user_command) == 0)
    {
        if (current_mode == DUMB_CMD_OPNBX)
        {
            printf("Error: Only 'put', 'next' and 'close' commands are accepted in 'OPEN' mode.\n");
            return USR_CMD_STATUS_BAD_CMD;
        }
        char buffer[READ_BUFFER_LEN];
        printf("Okay, create which message box?\n");
        do
        {
            printf("create:> ");
            char *user_cmd = fgets(buffer, READ_BUFFER_LEN, stdin);
            if (*user_cmd == '\n')
            {
                continue;
            }
            cmd_args = user_cmd;
            char *ending_space = strchr(cmd_args, '\n');
            if (ending_space)
                *ending_space = 0;
            int len = strlen(user_cmd);
            if (5 > len || len > 25)
            {
                printf("[%u] Error: Command was unsuccessful, MB len should be 5 to 25 letter long. Actual: %d\n", __LINE__, len);
                return USR_CMD_STATUS_BAD_CMD;
            }
            break;
        }
        while (1);
        current_mode = command->opcode = DUMB_CMD_CREAT;
        DUMP_ARRAY((char*) command, 32);
        strcpy(command->command, "CREAT");
        if (cmd_args)
            strncpy(command->args, cmd_args, ARGS_LEN_MAX);
    }
    else if (strcmp("delete", user_command) == 0)
    {
        if (current_mode == DUMB_CMD_OPNBX)
        {
            printf("Error: Only 'put', 'next' and 'close' commands are accepted in 'OPEN' mode.\n");
            return USR_CMD_STATUS_BAD_CMD;
        }

        char buffer[READ_BUFFER_LEN];
        printf("Okay, delete which message box?\n");
        do
        {
            printf("delete:> ");
            char *user_cmd = fgets(buffer, READ_BUFFER_LEN, stdin);
            if (*user_cmd == '\n')
            {
                continue;
            }
            cmd_args = user_cmd;
            char *ending_space = strchr(cmd_args, '\n');
            if (ending_space)
                *ending_space = 0;
            int len = strlen(user_cmd);
            if (5 > len || len > 25)
            {
                printf("[%u] Error: Command was unsuccessful, MB len should be 5 to 25 letter long. Actual: %d\n", __LINE__, len);
                return USR_CMD_STATUS_BAD_CMD;
            }
            break;
        }
        while (1);
        current_mode = command->opcode = DUMB_CMD_DELBX;
        DUMP_ARRAY((char*) command, 32);
        strcpy(command->command, "DELBX");
        if (cmd_args)
            strncpy(command->args, cmd_args, ARGS_LEN_MAX);
    }
    else if (strcmp("open", user_command) == 0)
    {
        if (current_mode == DUMB_CMD_OPNBX)
        {
            printf("Error: Only 'put', 'next' and 'close' commands are accepted in 'OPEN' mode.\n");
            return USR_CMD_STATUS_BAD_CMD;
        }

        char buffer[READ_BUFFER_LEN];
        printf("Okay, open which message box?\n");
        do
        {
            printf("open:> ");
            char *user_cmd = fgets(buffer, READ_BUFFER_LEN, stdin);
            if (*user_cmd == '\n')
            {
                continue;
            }
            cmd_args = user_cmd;
            char *ending_space = strchr(cmd_args, '\n');
            if (ending_space)
                *ending_space = 0;
            int len = strlen(user_cmd);
            if (5 > len || len > 25)
            {
                printf("[%u] Error: Command was unsuccessful, MB len should be 5 to 25 letter long. Actual: %d\n", __LINE__, len);
                return USR_CMD_STATUS_BAD_CMD;
            }
            break;
        }
        while (1);
        current_mode = command->opcode = DUMB_CMD_OPNBX;
        DUMP_ARRAY((char*) command, 32);
        strcpy(command->command, "OPNBX");
        if (cmd_args)
        {
            strncpy(command->args, cmd_args, ARGS_LEN_MAX);
            strcpy(open_mailbox, cmd_args);
        }
    }
    else if (strcmp("close", user_command) == 0)
    {
        if (current_mode != DUMB_CMD_OPNBX)
        {
            printf("Error: Nothing to be closed.\n");
            return USR_CMD_STATUS_BAD_CMD;
        }

        current_mode = command->opcode = DUMB_CMD_CLSBX;
        DUMP_ARRAY((char*) command, 32);
        strcpy(command->command, "CLSBX");
        if (cmd_args)
            strncpy(command->args, open_mailbox, ARGS_LEN_MAX);
        open_mailbox[0] = 0;
    }
    else if (strcmp("next", user_command) == 0)
    {
        if (current_mode != DUMB_CMD_OPNBX)
        {
            printf("Error:  Neither Mailbox is  opened.\n");
            return USR_CMD_STATUS_BAD_CMD;
        }
        command->opcode = DUMB_CMD_NXTMG;
        DUMP_ARRAY((char*) command, 32);
        strcpy(command->command, "NXTMG");
        command->args[0] = 0;
    }
    else if (strcmp("put", user_command) == 0)
    {
        if (current_mode != DUMB_CMD_OPNBX)
        {
            printf("Error:  Neither Mailbox is  opened.\n");
            return USR_CMD_STATUS_BAD_CMD;
        }
        char buffer[READ_BUFFER_LEN];
        printf("Okay, put which message?\n");
        do
        {
            printf("put:> ");
            char *user_cmd = fgets(buffer, READ_BUFFER_LEN, stdin);
            if (*user_cmd == '\n')
            {
                continue;
            }
            int len = strlen(user_cmd);
            if (len > 255)
            {
                ERR_PRINT("Command was unsuccessful, Message len shouldn't exceed 255 ASCII letters\n");
                return USR_CMD_STATUS_BAD_CMD;
            }
            cmd_args = user_cmd;
            char *ending_space = strchr(cmd_args, '\n');
            if (ending_space)
                *ending_space = 0;
            break;
        }
        while (1);
        command->opcode = DUMB_CMD_PUTMG;
        DUMP_ARRAY((char*) command, 32);
        strcpy(command->command, "PUTMG");
        if (cmd_args)
        {
            snprintf(command->args, ARGS_LEN_MAX, "!%d!%s", strlen(cmd_args), cmd_args);
        }
        else
        {
            snprintf(command->args, ARGS_LEN_MAX, "!0!");
        }
    }
    else if (strcmp("help", user_command) == 0)
    {
        printf("I'am here - > HELP\n");
        print_help_menu();
    }
    else
    {
        ERR_PRINT("That is not a command, for a command list enter 'help'.");
        return USR_CMD_STATUS_BAD_CMD;
    }
    return USR_CMD_STATUS_OK;
}

void
print_help_menu()
{

    printf("\t----MAIN MENU-----\n");
    printf("\tPlease use one of the following commands:\n");
    printf("\t1. quit   - stop session and exit the program\n");
    printf("\t2. create - Create new massage box on the server\n");
    printf("\t3. delete - will exit the program\n");
    printf("\t4. open   - open if massage box if exists\n");
    printf("\t5. close  - close current massage box\n");
    printf("\t6. next   - Get the next massage from the current massage box\n");
    printf("\t7. put    - Put a message in to the currently open message box\n");
    printf("\t8. help   - will show main menu\n");
}

int send_to_server(dumb_command_t *command, int sockfd, char *buffer);
int send_cmd_to_server(dumb_command_t *command, int sockfd);
int read_from_server(dumb_command_t *command, int sockfd, char *buffer);

int
main(int argc, char *argv[])
{

    int sockfd, portno, n, counter = 0, con = 0;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[READ_BUFFER_LEN];

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
        fprintf(stderr, "Failed to connect to DUMB server\n");
        sleep(1);
    }
    if (con)
    {
        error("Failed to connect. Exit Now\n");
    }

    dumb_command_t command = {0};
    command.opcode = DUMB_CMD_HELLO;
    send_to_server(&command, sockfd, hello);
    bzero(buffer, READ_BUFFER_LEN);
    read_from_server(&command, sockfd, buffer);
    printf("%s\n", buffer);

    while (1)
    {
        //clean buffer
        if (current_mode == DUMB_CMD_OPNBX)
            printf("%s:> ", open_mailbox);
        else
            printf(":> ");
        bzero(buffer, READ_BUFFER_LEN);
        char *user_cmd = fgets(buffer, READ_BUFFER_LEN, stdin);
        if (*user_cmd == '\n')
        {
            continue;
        }
        usr_cmd_parse_status status = parse_read(user_cmd, &command);

        DEBUG_PRINT("status %u opcode: %u\n", status, command.opcode);

        if (USR_CMD_STATUS_BAD_CMD == status)
        {
            continue;
        }
        else if (USR_CMD_STATUS_BAD_ARGS == status)
        {
            continue;
        }
        else if (USR_CMD_STATUS_OK == status)
        {
            switch (command.opcode)
            {
                case DUMB_CMD_HELLO:
                    printf("Hello State\n");
                    break;
                case DUMB_CMD_GDBYE:
                {
                    send_cmd_to_server(&command, sockfd);
                    bzero(buffer, READ_BUFFER_LEN);
                    if (read_from_server(&command, sockfd, buffer) > 0)
                    {
                        printf("Error: Unexpected Server behavior. Closing....\n");
                        exit(1);
                    }
                }
                    break;
                case DUMB_CMD_CREAT:
                {
                    send_cmd_to_server(&command, sockfd);
                    bzero(buffer, READ_BUFFER_LEN);
                    read_from_server(&command, sockfd, buffer);
                    DEBUG_PRINT("%s\n", buffer);
                    int res = strcmp(buffer, "OK!");
                    if (0 == res)
                    {
                        printf("Success! Message box '%s' is created now.\n", command.args);
                        break;
                    }
                    res = strcmp(buffer, "ER:EXIST");
                    if (0 == res)
                    {
                        printf("Error. Message box '%s' already exists.\n", command.args);
                        break;
                    }
                    res = strcmp(buffer, "ER:WHAT?");
                    if (0 == res)
                    {
                        printf("Error. Your message is in some way broken or malformed.\n");
                        break;
                    }
                }
                    break;
                case DUMB_CMD_DELBX:
                {
                    send_cmd_to_server(&command, sockfd);
                    bzero(buffer, READ_BUFFER_LEN);
                    read_from_server(&command, sockfd, buffer);
                    DEBUG_PRINT("%s\n", buffer);
                    int res = strcmp(buffer, "OK!");
                    if (0 == res)
                    {
                        printf("Success! Message box '%s' is now deleted.\n", command.args);
                        break;
                    }
                    res = strcmp(buffer, "ER:NEXST");
                    if (0 == res)
                    {
                        printf("Error. Message box '%s' does not exist.\n", command.args);
                        break;
                    }
                    res = strcmp(buffer, "ER:OPEND");
                    if (0 == res)
                    {
                        printf("Error. Message box '%s' opened and can't be deleted.\n", command.args);
                        break;
                    }
                    res = strcmp(buffer, "ER:WHAT?");
                    if (0 == res)
                    {
                        printf("Error. Your message is in some way broken or malformed.\n");
                        break;
                    }
                }
                    break;
                case DUMB_CMD_OPNBX:
                {
                    send_cmd_to_server(&command, sockfd);
                    bzero(buffer, READ_BUFFER_LEN);
                    read_from_server(&command, sockfd, buffer);
                    DEBUG_PRINT("%s\n", buffer);
                    current_mode = DUMB_CMD_HELLO;

                    int res = strcmp(buffer, "OK!");

                    if (0 == res)
                    {
                        printf("Success! Message box '%s' is now open.\n", command.args);
                        current_mode = DUMB_CMD_OPNBX;
                        break;
                    }
                    res = strcmp(buffer, "ER:NEXST");
                    if (0 == res)
                    {
                        printf("Error. Message box '%s' does not exist.\n", command.args);
                        break;
                    }
                    res = strcmp(buffer, "ER:OPEND");
                    if (0 == res)
                    {
                        printf("Error. Message box can't be opened. It is opened by another client.\n");
                        break;
                    }
                    res = strcmp(buffer, "ER:WHAT?");
                    if (0 == res)
                    {
                        printf("Error. Your message is in some way broken or malformed.\n");
                        break;
                    }

                }
                    break;
                case DUMB_CMD_CLSBX:
                {
                    send_cmd_to_server(&command, sockfd);
                    bzero(buffer, READ_BUFFER_LEN);
                    read_from_server(&command, sockfd, buffer);
                    DEBUG_PRINT("%s\n", buffer);
                    int res = strcmp(buffer, "OK!");
                    if (0 == res)
                    {
                        printf("Success! Message box '%s' is now closed.\n", command.args);
                        break;
                    }
                    res = strcmp(buffer, "ER:NEXST");
                    if (0 == res)
                    {
                        printf("Error. Message box '%s' does not exist.\n", command.args);
                        break;
                    }
                    res = strcmp(buffer, "ER:NOOPN");
                    if (0 == res)
                    {
                        printf("Error. Message box '%s' either not opened or doesn't exist.\n", command.args);
                        break;
                    }
                    res = strcmp(buffer, "ER:WHAT?");
                    if (0 == res)
                    {
                        printf("Error. Your message is in some way broken or malformed.\n");
                        break;
                    }
                }
                    break;
                case DUMB_CMD_NXTMG:
                {
                    send_cmd_to_server(&command, sockfd);
                    bzero(buffer, READ_BUFFER_LEN);
                    read_from_server(&command, sockfd, buffer);
                    DEBUG_PRINT("%s\n", buffer);
                    int res = check_if_ascii(buffer);
                    if (0 == res)
                    {
                        printf("[%u]: Error: Malformed response.\n", __LINE__);
                        break;
                    }
                    res = strcmp(buffer, "ER:EMPTY");
                    if (0 == res)
                    {
                        printf("No more messages in the mailbox.\n");
                        break;
                    }
                    res = strcmp(buffer, "ER:NOOPN");
                    if (0 == res)
                    {
                        printf("Error. Message box either not opened or doesn't exist.\n");
                        break;
                    }
                    res = strcmp(buffer, "ER:WHAT?");
                    if (0 == res)
                    {
                        printf("Error. Your message is in some way broken or malformed.\n");
                        break;
                    }

                    char *ok_string = strstr(buffer, "OK!");
                    if (!ok_string)
                    {
                        printf("[%u]: Error: Malformed response.\n", __LINE__);
                        break;
                    }
                    else
                    {
                        char *args = strchr(ok_string, '!');
                        if (!args)
                        {
                            printf("[%u]: Error: Malformed response.\n", __LINE__);
                            break;
                        }
                        *args = 0;
                        args++;
                        char *message = strchr(args, '!');
                        if (!message)
                        {
                            printf("Error: Malformed response.\n");
                            break;
                        }
                        *message = 0;
                        message++;
                        unsigned int expected_msg_len = atoi(args);
                        unsigned int actual_msg_len = strlen(message);
                        if (expected_msg_len > 255)
                        {
                            printf("Error: The message is too long.  %u (Max = 255)\n", expected_msg_len);
                            break;
                        }
                        if (expected_msg_len != actual_msg_len)
                        {
                            printf("Error: incorrect length of the message: Expected: %u. Actual: %u\n", expected_msg_len, actual_msg_len);
                            break;
                        }
                        printf("MSG: %s\n", message);
                        break;
                    }
                }
                    break;
                case DUMB_CMD_PUTMG:
                {
                    send_cmd_to_server(&command, sockfd);
                    bzero(buffer, READ_BUFFER_LEN);
                    read_from_server(&command, sockfd, buffer);
                    DEBUG_PRINT("%s\n", buffer);
                    DUMP_ARRAY(buffer, 17);
                    char *ok_string = strstr(buffer, "OK!");
                    if (ok_string)
                    {
                        printf("Success! The message has been added to the mailbox\n");
                        break;
                    }
                    int res = strcmp(buffer, "ER:NOOPN");
                    if (0 == res)
                    {
                        printf("Error. Message box is not accessible.\n");
                        break;
                    }
                    res = strcmp(buffer, "ER:WHAT?");
                    if (0 == res)
                    {
                        printf("Error. Your message is in some way broken or malformed.\n");
                        break;
                    }
                    break;
                }
            }
            continue;
        }
        else
        {
            continue;
        }
    }

    close(sockfd);
    return 0;
}

int
send_to_server(dumb_command_t *command, int sockfd, char *buffer)
{
    int n = write(sockfd, buffer, strlen(buffer));
    DEBUG_PRINT("%i bytes sent to server\n", n);
    if (n < 0)
    {
        error("Error on reading");
    }
    return n;
}

int
send_cmd_to_server(dumb_command_t *command, int sockfd)
{
    char cmd_buffer[128] = {0};
    int n = sprintf(cmd_buffer, "%s", command->command);
    if (command->args[0] != 0)
        n += sprintf(cmd_buffer + n, " %s", command->args);
    return send_to_server(command, sockfd, cmd_buffer);
}

int
read_from_server(dumb_command_t *command, int sockfd, char *buffer)
{
    //read massages from the server/ holds massages back from the server...
    int n = read(sockfd, buffer, READ_BUFFER_LEN);
    if (n < 0)
    {
        error("Error on reading");
    }
    if (n == 0)
    {
        printf("Socket can not be read from and was closed on server side\n");
        exit(0);
    }

    DEBUG_PRINT("%i %s\n", n, buffer);
    return n;
}
