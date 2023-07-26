#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#define BUFF_SIZE 1024
#define MAX_EVENTS 16

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

    ret = epoll_create1(0);
    error_check(ret, "create epoll");

    int epollfd = ret;

    struct epoll_event ev;

    ev.events = EPOLLIN;
    ev.data.fd = sockfd;

    ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev);
    error_check(ret, "epoll add listening socket");

    printf("listening socket: %d\n", sockfd);

    for(;;)
    {
        struct epoll_event events[MAX_EVENTS];

        ret = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        error_check(ret, "epoll wait");

        int events_count = ret;

        for (int i = 0; i != events_count; ++i)
        {
            int fd = events[i].data.fd;

            if (fd == sockfd)
            {
                struct sockaddr_in cliaddr;
                socklen_t len = sizeof(cliaddr);

                ret = accept(fd, (struct sockaddr*)&cliaddr, &len);
                error_check(ret, "accept connection");

                int connfd = ret;

                printf("connected socket: %d\n", connfd);

                ret = fcntl(connfd, F_SETFL, O_NONBLOCK);
                error_check(ret, "fcntl set nonblock flag");

                ev.data.fd = connfd;
                ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev);
                error_check(ret, "epoll add connection socket");
            }
            else
            {
                char buff[BUFF_SIZE];

                ret = recv(fd, buff, sizeof(buff), MSG_DONTWAIT);
                error_check(ret, "recv");

                if (ret == 0)
                {
                    printf("disconnected socket: %d\n", fd);
                    close(fd);
                    break;
                }

                int recv_size = ret;

                ret = send(fd, buff, recv_size, 0);
                error_check(ret, "send");

                if (ret != recv_size)
                {
                    printf("error socket: %d recieved message size: %d sent message size: %d\n", fd, recv_size, ret);
                    close(fd);
                    break;
                }
            }
        }
    }

    close(sockfd);
    close(epollfd);
}
