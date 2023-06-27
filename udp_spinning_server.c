#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define BUFF_SIZE 1024

static __inline__ void error_check(int ret, const char* text)
{
    if (ret == -1)
    {
        printf("error %s: %s\n", text, strerror(errno));
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    in_addr_t addr = htonl(INADDR_ANY);
    in_port_t port = htons(8080);

    if (argc > 1)
        addr = inet_addr(argv[1]);

    if (argc > 2)
        port = htons(atoi(argv[2]));

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    error_check(sockfd, "create socket");

    struct sockaddr_in servaddr;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = addr;
    servaddr.sin_port = port;

    int ret = bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    error_check(ret, "bind socket");

    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    for (;;)
    {
        struct sockaddr_in cliaddr;
        socklen_t len = sizeof(cliaddr);
        char buff[BUFF_SIZE];

        do
        {
            ret = recvfrom(sockfd, buff, BUFF_SIZE, MSG_DONTWAIT, (struct sockaddr*)&cliaddr, &len);
        }
        while (ret == -1 && errno == EAGAIN);

        error_check(ret, "recvfrom");

        int recv_size = ret;

        do
        {
            ret = sendto(sockfd, buff, recv_size, MSG_DONTWAIT, (struct sockaddr*)&cliaddr, len);
        }
        while (ret == -1 && errno == EAGAIN);

        error_check(ret, "sendto");

        if (ret != recv_size)
        {
            printf("recieved datagram size: %d sent datagram size: %d\n", recv_size, ret);
            exit(1);
        }
    }

    close(sockfd);
}