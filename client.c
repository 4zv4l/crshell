#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include "./socket.c"

#define MSG_FLAG MSG_NOSIGNAL

/*
 * read char by char
 * stop reading after reaching 0
*/
static void
read_result(int fd)
{
    char c;
    while(recv(fd, &c, 1, MSG_FLAG) > 0) {
        if (c == 0) break;
        printf("%c", c);
    }
}

/*
 * send the command to the server
 * basically send the command + 0 as terminator
*/
static int
send_command(int fd, char *cmd)
{
    // strlen safe here since it comes from readline
    if (send(fd, cmd, strlen(cmd)+1, MSG_FLAG) < 0) return 0;
    return 1;
}

extern int
main(int argc, char **argv)
{
    if (argc != 3) return printf("usage: %s [ip] [port]\n", argv[0]);
    char *ip = argv[1];
    unsigned short int port = atoi(argv[2]);

    int conn = tcp_connect(ip, port);
    if (!conn) return perror(""), 1;
    printf("Connected to %s:%d\n", ip, port);

    for(;;) {
        char *cmd = readline("$ ");
        if (!cmd || !cmd[0]) continue;
        if (!send_command(conn, cmd)) break;
        if (!strcmp("exit", cmd) || !strcmp("stop", cmd)) return 0;
        free(cmd);
        read_result(conn);
    }
    close(conn);
}
