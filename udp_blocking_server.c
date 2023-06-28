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

#include "common.h"

#define BUFF_SIZE 1024

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

    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(cliaddr);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = addr;
    servaddr.sin_port = port;

    int ret = bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    error_check(ret, "bind socket");

    for (;;)
    {
        char buff[BUFF_SIZE];

        ret = recvfrom(sockfd, buff, BUFF_SIZE, 0, (struct sockaddr*)&cliaddr, &len);

        error_check(ret, "recvfrom");

        int recv_size = ret;

        ret = sendto(sockfd, buff, recv_size, 0, (struct sockaddr*)&cliaddr, len);

        error_check(ret, "sendto");

        if (ret != recv_size)
        {
            printf("recieved datagram size: %d sent datagram size: %d\n", recv_size, ret);
            exit(1);
        }
    }

    close(sockfd);
}
