#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "./socket.c"

#define DEFAULT_BUFF 1024
#define ERROR_CMD "failed to execute\n"
#define CD_CMD "changed dir with success\n"
#define MSG_FLAG MSG_NOSIGNAL

/*
 * read char by char
 * realloc if too much data is sent
 * caller own the memory (need to be freed)
 * stop reading after reaching 0
*/
static char *
read_command(int fd, int *length)
{
    int size = DEFAULT_BUFF;
    char *buff = malloc(size);
    if (!buff) return 0;
    char c;
    for (*length = 0; *length <= size && recv(fd, &c, 1, MSG_FLAG) > 0 && c != 0; (*length)++) {
        if (*length >= size) {
            buff = realloc(buff, size + DEFAULT_BUFF);
            size += DEFAULT_BUFF;
            if (!buff) return 0;    
        }
        buff[*length] = c;
    }
    buff[*length] = 0;
    return buff;
}

/*
 * popen() the command
 * read char by char
 * send char by char
 * send 0 as stop signal
*/
enum cmd {
    CMD_OK,
    CMD_FAILED,
    CMD_EXIT
};
static enum cmd
execute_send_command(int fd, char *cmd)
{
    // handle exit command
    if (!strcmp("exit", cmd)) return CMD_EXIT;

    // handle cd command
    if (!strncmp("cd", cmd, 2)) {
        int len = strlen(cmd);
        if (len < 4) return CMD_FAILED;
        if (chdir(cmd+3) != 0) return CMD_FAILED;
        if (send(fd, CD_CMD, strlen(CD_CMD)+1, MSG_FLAG) <= 0) return CMD_EXIT;
        return CMD_OK;
    }

    // do popen stuff
    FILE *f = popen(cmd, "r");
    if (!f) return 0;

    char c;
    while((c = getc(f)) != EOF) {
        if (c == 0) c = 1;
        if (send(fd, &c, 1, MSG_FLAG) <= 0) return CMD_EXIT;
    }

    // check return code before sending 0 to terminate
    int ret = pclose(f);
    if (ret != 0) return CMD_FAILED;

    c = 0;
    if (send(fd, &c, 1, MSG_FLAG) <= 0) return CMD_EXIT;

    return CMD_OK;
}

static int
handle_client(int serv, int client)
{
    for(;;) {
        int len = 0;
        char *cmd = read_command(client, &len);
        if (!cmd || !len) break;
        printf("=> %s\n", cmd);

        // handle special command
        if (!strcmp("exit", cmd)) {
            free(cmd);
            break;
        }
        if (!strcmp("stop", cmd)) {
            free(cmd);
            exit(0);
        }

        int ok = execute_send_command(client, cmd);
        free(cmd);

        // strlen is safe here since hard coded
        if (ok == CMD_FAILED) {
            if (send(client, ERROR_CMD, strlen(ERROR_CMD)+1, MSG_FLAG) <= 0) break;
        }
        if (ok == CMD_EXIT) break;
    }
    return 0;
}

extern int
main(int argc, char **argv)
{
    if (argc != 3) return printf("usage: %s [ip] [port]\n", argv[0]), 1;
    char *ip = argv[1];
    unsigned short int port = atoi(argv[2]);

    int serv = tcp_listen(ip, port);
    if (!serv) return perror(""), 2;
    printf("Listening on %s:%d\n", ip, port);

    for(;;) {
        printf("waiting..\n");
        int client = tcp_accept(serv);
        if (client < 0) continue;
        printf("Got client !\n");

        handle_client(serv, client);
        
        close(client);
        printf("bye\n");
    }
}
