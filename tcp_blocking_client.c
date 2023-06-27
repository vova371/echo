#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

static __inline__ uint64_t checkpoint()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

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
    in_addr_t addr = htonl(INADDR_LOOPBACK);
    in_port_t port = htons(8080);

    if (argc > 1)
        addr = inet_addr(argv[1]);

    if (argc > 2)
        port = htons(atoi(argv[2]));

    int count = 1000;

    if (argc > 3)
        count = atoi(argv[3]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    error_check(sockfd, "create socket");

    struct sockaddr_in servaddr;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = addr;
    servaddr.sin_port = port;

    int ret = connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    error_check(ret, "bind socket");

    uint64_t n0 = checkpoint();

    for (int i=0; i<count; ++i)
    {
        int64_t value = i;

        ret = send(sockfd, &value, sizeof(value), MSG_DONTWAIT);

        error_check(ret, "send");

        if (ret != sizeof(value))
        {
            printf("sent message size: %d expected: %lu\n", ret, sizeof(value));
            exit(1);
        }

        int sent_size = ret;
        value = 0;

        do
        {
            ret = recv(sockfd, &value, sizeof(value), MSG_DONTWAIT);
        }
        while (ret == -1 && errno == EAGAIN);

        error_check(ret, "recv");

        // Assuming message wasn't fragmented
        if (ret != sent_size)
        {
            printf("sent message size: %d received message size: %d\n", sent_size, ret);
            exit(1);
        }

        if (value != i)
        {
            printf("incorrect value received\n");
            exit(1);
        }
    }

    uint64_t n1 = checkpoint();

    close(sockfd);

    printf("send/recv %d values in %ldns rate %ld\n", count, n1 - n0, (n1 - n0) / count);
}
