#include <sys/epoll.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>


#define MAXBUF 1000
#define MAXEVENTS 5
#define PORTNUM 8888

void eperror(char * s)
{
    printf("in perror\n");
    perror(s);
    exit(errno);
};

int main(int argc, char *argv[])
{
    int epfd, ready, s, j, numOpenFDs;
    struct epoll_event ev;
    struct epoll_event evlist[MAXEVENTS];
    char buf[MAXBUF];
    /////
    struct sockaddr_in svaddr, claddr;
    socklen_t len;
    int sfd;

    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sfd == -1)
        eperror("socket");
    memset(&svaddr, 0, sizeof(struct sockaddr_in));
    svaddr.sin_family = AF_INET;
    svaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    svaddr.sin_port = htons(PORTNUM);

    if(bind(sfd, (struct sockaddr *) &svaddr, sizeof(struct sockaddr_in)) == -1)
        eperror("bind");

    epfd = epoll_create(1);
    if(epfd == -1)
        eperror("epoll create");

    for(j = 0; j < 1; j++){
        // fd = open(argv[j], O_RDONLY);
        // if(fd == -1)
        //     eperror("open");
        // printf("Opened \"%s\" on fd %d \n", argv[j], fd);

        ev.events = EPOLLIN;
        ev.data.fd = sfd;
        if(epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &ev) == -1)
            eperror("epoll_ctl");
    }
    numOpenFDs = 1;
    while(numOpenFDs > 0) {
        printf("%s\n", "about to epoll_wait");
        ready = epoll_wait(epfd, evlist, MAXEVENTS, -1);
        if(ready == -1) {
            if(errno == EINTR)
                continue;
            else
                eperror("epoll_wait");
        }
        printf("ready: %d\n", ready);

        for(j=0; j< ready;j++){
            printf(" fd=%d; events: %s%s%s\n", evlist[j].data.fd,
                (evlist[j].events & EPOLLIN) ? "EPOLLIN " : "",
                (evlist[j].events & EPOLLHUP) ? "EPOLLHUP " : "",
                (evlist[j].events & EPOLLERR) ? "EPOLLERR " : "");

            if(evlist[j].events & EPOLLIN){
                // s = read(evlist[j].data.fd, buf, MAXBUF);
                len = sizeof(struct sockaddr_in);
                s = recvfrom(sfd, buf, MAXBUF, 0, (struct sockaddr *) &claddr, &len);
                if (s == -1)
                    eperror("read");
                printf("   read %d bytes: %.*s\n", s, s, buf );
                s = sendto(sfd, buf, s, 0, (struct sockaddr *) &claddr, len);
                if (s == -1)
                    eperror("read");
                printf("   sent back\n");
            }
            else if(evlist[j].events & (EPOLLHUP | EPOLLERR)){
                printf("    closing fd %d \n", evlist[j].data.fd);
                if(close(evlist[j].data.fd) == -1)
                    eperror("close");
                numOpenFDs--;
            }
        }
    }

    printf("All fd closed, bye\n" );
    exit(0);
}
