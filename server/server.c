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
static size_t
execute(char* cmd, size_t len, int connfd) {

    // allow to change the process directory
    if(strncmp(cmd, "cd", 2) == 0) {
        char *path = cmd+3; // cmd+3 => skip this ['c', 'd', ' '] /!\ pointers
        if(chdir(path)!=0) {
            return 0;
        }
        size_t l = snprintf(cmd, MAX, "changed dir with success\n");
        write(connfd, cmd, l);
        write(connfd, EOC, sizeof(EOC));
        return 1;
    }

    // open a pipe running the command
    FILE* fp = popen(cmd, "r");
    if (!fp) {
        return 0;
    }

    // count how many char is read
    size_t counter = 0;

    // read stdout from pipe
    // send it to the client
    int c;
    char buff[MAX] = {0};
    while((c = fgetc(fp))!=EOF) {
        buff[counter] = c;
        counter += 1;
        if(counter == MAX) {
            write(connfd, buff, counter);
            counter = 0;
        }
    }
    // write what remains in the buffer
    if(counter > 0)
        write(connfd, buff, counter);

    // send End Of Command
    write(connfd, EOC, sizeof(EOC));

    // close the pipe
    pclose(fp);

    // return how many bytes read
    return 1;
}

// Function designed for chat between client and server.
static int
handle(int connfd) {
    char buff[MAX] = {0};
    size_t bytes;
    // infinite loop for chat
    for (;;) {
        // read the message from client and copy it in buffer
        bytes = read(connfd, buff, sizeof(buff));
        if(bytes == 0) {
            printf("client left the conn\n");
            close(connfd);
            return 0;
        }

        // if buff equals exit then close the conn
        if (strncmp("exit", buff, 4) == 0) {
            printf("client left the conn\n");
            close(connfd);
            return 0;
        }

        // if buff equals STOP then close the conn
        if (strncmp("STOP", buff, 4) == 0) {
            printf("client left the conn\n");
            close(connfd);
            return 1;
        }
        // print buffer which contains the client contents
        printf("%.*s\n", (int)bytes, buff);

        // execute the command in buff
        // send the output to connfd
        buff[bytes] = 0;
        int rc = execute(buff, bytes, connfd);
        if(!rc) {
            char error[] = "error when running command\n" EOC;
            write(connfd, error, sizeof(error));
        }
    }
    return 0;
}

extern int
main(int argc, char **argv){
    // check arguments
    if(argc != 3) {
        printf("Usage: %s [ip] [port]\n", argv[0]);
        return 1;
    }

    const char* ip = argv[1];
    const int port = atoi(argv[2]);

    int sockfd = listenAddr(ip, port);
    if(sockfd == -1) {
        perror("");
        exit(1);
    }

    int connfd;
    struct sockaddr cli = {0};
    socklen_t len = sizeof(cli);
    // loop accepting client
    while((connfd = accept(sockfd, (struct sockaddr*)&cli, &len))) {
        // if accepting fail, just wait for another client
        if (connfd < 0) {
            perror("");
            continue;
        }
        printf("got a client !\n");

        // handle client
        // 0 			=> not shut down
        // 1 or beyond 	=> shut down
        if(handle(connfd)){break;}
    }

    // close the server socket
    close(sockfd);
}
