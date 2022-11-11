#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h> // better input system
#include "../lib/easysocket.h"
#define MAX 4048 // 4kb buffer

void handle(int sockfd) {
    char *input;
    char buff[MAX];
    ssize_t bytes;
    for (;;) {
        // get user input
        input = readline("$ ");

        // send cmd to socket
        write(sockfd, input, strlen(input));

        // if send exit then close
        if ((strncmp(input, "exit", 4)) == 0) {
            printf("exiting...\n");
            free(input);
            break;
        }

        // if send STOP then close
        if ((strncmp(input, "STOP", 4)) == 0) {
            printf("shutting down the server...\n");
            free(input);
            break;
        }

        // free the non needed input
        free(input);

        // read cmd output from socket
        bytes = read(sockfd, buff, sizeof(buff));
        printf("%.*s", (int)bytes, buff);
    }
}

int main(int argc, char **argv) {
    // check arguments
    if (argc != 3) {
        printf("Usage: %s [ip] [port]\n", argv[0]);
        return 1;
    }

    const char* ip = argv[1];
    const int port = atoi(argv[2]);

    int sockfd = connectAddr(ip, port);
    if(sockfd == -1) {
        perror("");
        exit(-1);
    }
    printf("connected to the server..\n");

    // function for chat
    handle(sockfd);

    // close the socket
    close(sockfd);
}
