#include <sys/epoll.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define MAXBUF 1000
#define MAXEVENTS 5

int main(int argc, char *argv[])
{
    int epfd, ready, fd, s, j, numOpenFDs;
    struct epoll_event ev;
    struct epoll_event evlist[MAXEVENTS];
    char buf[MAXBUF];

    if(argc < 2 || strcmp(argv[1], "--help") == 0)
        fprintf(stderr, "Usage: %s %s\n", argv[0], "file ...");

    epfd = epoll_create(argc - 1);
    if(epfd == -1)
        perror("epoll create");

    for(j = 1; j < argc; j++){
        fd = open(argv[j], O_RDONLY);
        if(fd == -1)
            perror("open");
        printf("Opened \"%s\" on fd %d \n", argv[j], fd);

        ev.events = EPOLLIN;
        ev.data.fd = fd;
        if(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1)
            perror("epoll_ctl");
    }
    numOpenFDs = argc - 1;
    while(numOpenFDs > 0) {
        printf("%s\n", "about to epoll_wait");
        ready = epoll_wait(epfd, evlist, MAXEVENTS, -1);
        if(ready == -1) {
            if(errno == EINTR)
                continue;
            else
                perror("epoll_wait");
        }
        printf("ready: %d\n", ready);

        for(j=0; j< ready;j++){
            printf(" fd=%d; events: %s%s%s\n", evlist[j].data.fd,
                (evlist[j].events & EPOLLIN) ? "EPOLLIN " : "",
                (evlist[j].events & EPOLLHUP) ? "EPOLLHUP " : "",
                (evlist[j].events & EPOLLERR) ? "EPOLLERR " : "");

            if(evlist[j].events & EPOLLIN){
                s = read(evlist[j].data.fd, buf, MAXBUF);
                if (s == -1)
                    perror("read");
                printf("   read %d bytes: %.*s\n", s, s, buf );
            }
            else if(evlist[j].events & (EPOLLHUP | EPOLLERR)){
                printf("    closing fd %d \n", evlist[j].data.fd);
                if(close(evlist[j].data.fd) == -1)
                    perror("close");
                numOpenFDs--;
            }
        }
    }

    printf("All fd closed, bye\n" );
    exit(0);
}
