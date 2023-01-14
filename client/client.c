#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h> // better input system
#include <readline/history.h>
#include "../lib/easysocket.h"
#define MAX 4096 // 4kb buffer
#define EOC "€$§"

static void
handle(int sockfd) {
    char *input;
    char buff[MAX];
    size_t bytes;
    for (;;) {
        input = readline("$ ");
        if(input[0] == 0) {
            free(input);
            continue;
        }
        add_history(input);
        write(sockfd, input, strlen(input));

        if ((strncmp(input, "exit", 4)) == 0) {
            printf("exiting...\n");
            free(input);
            break;
        }

        if ((strncmp(input, "STOP", 4)) == 0) {
            printf("shutting down the server...\n");
            free(input);
            break;
        }

        free(input);

        while(1) {
            bytes = read(sockfd, buff, sizeof(buff));
            buff[bytes] = 0;
            //printf("[+] got %ld bytes\n", bytes);
            //printf("[+] %.*s", (int)bytes, buff);
            char *ptr = strstr(buff, EOC);
            //if (!ptr) printf("[+] didn't send EOC yet\n");
            //if (ptr) printf("\n[+] got EOC : %p", ptr);
            if(!ptr) {
                printf("%.*s", (int)bytes, buff);    
            } else {
                // doesn't write the EOC
                int index = (ptr - buff);
                printf("%.*s", index, buff);
                break;
            }
        }
    }
}

extern int
main(int argc, char **argv) {
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

    using_history();

    handle(sockfd);

    HISTORY_STATE *myhist = history_get_history_state ();
    HIST_ENTRY **mylist = history_list ();
    for (int i = 0; i < myhist->length; i++) {
        free_history_entry(mylist[i]);
    }

    close(sockfd);
}
