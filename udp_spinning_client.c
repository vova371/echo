#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "common.h"

int main(int argc, char *argv[])
{
    in_addr_t addr = htonl(INADDR_LOOPBACK);
    in_port_t port = htons(8080);

    if (argc > 1)
        addr = inet_addr(argv[1]);

    if (argc > 2)
        port = htons(atoi(argv[2]));

    int count = 1000;

    if (argc > 3)
        count = atoi(argv[3]);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    error_check(sockfd, "create socket");

    struct sockaddr_in servaddr;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = addr;
    servaddr.sin_port = port;

    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    uint64_t n0 = checkpoint();

    for (int64_t i=0; i<count; ++i)
    {
        int64_t value = i;
        socklen_t len = sizeof(servaddr);
        int ret = 0;

        do
        {
            ret = sendto(sockfd, &value, sizeof(value), MSG_DONTWAIT, (struct sockaddr*)&servaddr, len);
        }
        while (ret == -1 && errno == EAGAIN);

        error_check(ret, "sendto");

        value = 0;
        int sent_size = ret;

        do
        {
            ret = recvfrom(sockfd, &value, sizeof(value), MSG_DONTWAIT, (struct sockaddr*)&servaddr, &len);
        }
        while (ret == -1 && errno == EAGAIN);

        error_check(ret, "recvfrom");

        if (ret != sent_size)
        {
            printf("sent datagram size: %d received datagram size: %d\n", sent_size, ret);
            exit(1);
        }

        if (value != i)
        {
            printf("sent value: %ld received value: %ld\n", i, value);
            exit(1);
        }
    }

    uint64_t n1 = checkpoint();

    close(sockfd);

    printf("send/recv %d messages in %ldns rate %ldns per message\n", count, n1 - n0, (n1 - n0) / count);
}
