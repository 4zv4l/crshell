#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

extern int
tcp_listen(char *ip, unsigned short int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (!fd) return 0;

    // force resue addr and port (quicker retry)
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    struct sockaddr_in  opts = {0};
    opts.sin_family = AF_INET;
    opts.sin_addr.s_addr = inet_addr(ip);
    opts.sin_port = htons(port);

    if (bind(fd, (struct sockaddr*)&opts, sizeof(opts)) < 0) return 0;

    if (listen(fd, 1) < 0) return 0;

    return fd;
}

extern int
tcp_accept(int fd)
{
    struct sockaddr_in info = {0};
    int len = sizeof(info);
    return accept(fd, (struct sockaddr*)&info, (socklen_t*)&len);
}


extern int
tcp_connect(char *ip, unsigned short int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (!fd) return 0;

    struct sockaddr_in  opts = {0};
    opts.sin_family = AF_INET;
    opts.sin_addr.s_addr = inet_addr(ip);
    opts.sin_port = htons(port);

    if (connect(fd, (struct sockaddr*)&opts, sizeof(opts)) < 0) return 0;

    return fd;
}
