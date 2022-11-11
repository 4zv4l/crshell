#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "./easysocket.h"

int listenAddr(const char *ip, int port) {
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1)
        return -1;

	int flag = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof flag);
	struct sockaddr_in server_setup;
	server_setup.sin_family = AF_INET;
	server_setup.sin_addr.s_addr = inet_addr(ip);
	server_setup.sin_port = htons(port);
	
    if(bind(sock, (struct sockaddr*)&server_setup, sizeof server_setup) == -1)
        return -1;
    
    if(listen(sock, 1) < 0)
        return -1;
	return sock;
}


int connectAddr(const char *ip, int port) {
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1)
        return -1;

	int flag = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof flag);
	struct sockaddr_in cli_setup;
	cli_setup.sin_family = AF_INET;
	cli_setup.sin_addr.s_addr = inet_addr(ip);
	cli_setup.sin_port = htons(port);
	
    if(connect(sock, (struct sockaddr*)&cli_setup, sizeof cli_setup) < 0)
        return -1;
	return sock;
}
