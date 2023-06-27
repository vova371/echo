#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
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

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    error_check(sockfd, "create socket");

    struct sockaddr_in servaddr;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = addr;
    servaddr.sin_port = port;

    int ret = bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    error_check(ret, "bind socket");

    ret = listen(sockfd, 8);

    error_check(ret, "listen socket");

    for(;;)
    {
        struct sockaddr_in cliaddr;
        socklen_t len = sizeof(cliaddr);

        int connfd = accept(sockfd, (struct sockaddr*)&cliaddr, &len);

        error_check(connfd, "accept connection");

        ret = fcntl(connfd, F_SETFL, O_NONBLOCK);

        error_check(ret, "fcntl set nonblock flag");

        for (;;)
        {
            char buff[BUFF_SIZE];

            do
            {
                ret = recv(connfd, buff, sizeof(buff), MSG_DONTWAIT);
            }
            while (ret == -1 && errno == EAGAIN);

            error_check(ret, "recv");

            if (ret == 0)
            {
                printf("client disconnected on recv\n");
                break;
            }

            int recv_size = ret;

            do
            {
                ret = send(connfd, buff, recv_size, MSG_DONTWAIT);
            }
            while (ret == -1 && errno == EAGAIN);

            error_check(ret, "send");

            if (ret == 0)
            {
                printf("client disconnected on send\n");
                break;
            }

            // Avoid fragmented messages
            if (ret != recv_size)
            {
                printf("received message size: %d sent message size: %d\n", recv_size, ret);
                exit(1);
            }
        }

        close(connfd);
    }

    close(sockfd);
}
