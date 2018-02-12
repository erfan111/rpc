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
#include <pthread.h>


#define MAXBUF 150
#define MAXEVENTS 10000
#define PORTNUM 8888
#define BACKLOG 1000
#define CORES 4

void eperror(char * s)
{
    printf("in perror\n");
    perror(s);
    exit(errno);
};

void event_listener(void*);

int main(int argc, char *argv[])
{
    int epfd, numOpenFDs;
    struct epoll_event ev;
    pthread_t thread_id;
    /////
    struct sockaddr_in svaddr;
    struct sockaddr_storage claddr;
    socklen_t addrlen;
    int sfd, cfd;

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sfd == -1)
        eperror("socket");
    memset(&svaddr, 0, sizeof(struct sockaddr_in));
    svaddr.sin_family = AF_INET;
    svaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    svaddr.sin_port = htons(PORTNUM);

    if(bind(sfd, (struct sockaddr *) &svaddr, sizeof(struct sockaddr_in)) == -1)
        eperror("bind");

    if( listen(sfd, BACKLOG) == -1)
    {
      fprintf(stderr, "%s\n", "Socket listening error");
      exit(-1);
    }

    epfd = epoll_create(1);
    if(epfd == -1)
        eperror("epoll create");

    if( pthread_create( &thread_id , NULL ,  event_listener , (void*) &epfd) < 0)
    {
        perror("could not create thread");
        return 1;
    }
    addrlen = sizeof(struct sockaddr_storage);
    numOpenFDs = 0;
    for(;;){
        cfd = accept(sfd, (struct sockaddr *)&claddr, &addrlen);
        if(cfd == -1){
          fprintf(stderr, "%s\n", "Socket accepting error");
          exit(-1);
        }
        ev.events = EPOLLIN;
        ev.data.fd = cfd;
        if(epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev) == -1)
            eperror("epoll_ctl");
        numOpenFDs++;
    }


    printf("All fd closed, bye\n" );
    exit(0);
}

void event_listener(void *arg)
{
    int epfd = *(int *)arg;
    int ready, j, s;
    socklen_t len;
    struct epoll_event evlist[MAXEVENTS];
    char buf[MAXBUF];
    while(1) {
        //printf("%s\n", "about to epoll_wait");
        ready = epoll_wait(epfd, evlist, MAXEVENTS, -1);
        if(ready == -1) {
            if(errno == EINTR)
                continue;
            else
                eperror("epoll_wait");
        }
        //printf("ready: %d\n", ready);

        for(j=0; j< ready;j++){
            //printf(" fd=%d; events: %s%s%s\n", evlist[j].data.fd,
            //    (evlist[j].events & EPOLLIN) ? "EPOLLIN " : "",
            //    (evlist[j].events & EPOLLHUP) ? "EPOLLHUP " : "",
            //    (evlist[j].events & EPOLLERR) ? "EPOLLERR " : "");

            if(evlist[j].events & EPOLLIN){
                // s = read(evlist[j].data.fd, buf, MAXBUF);
                len = sizeof(struct sockaddr_in);
                s = read(evlist[j].data.fd, buf, MAXBUF);
                if (s == -1)
                    eperror("read");
                //printf("   read %d bytes: %.*s\n", s, s, buf );
                s = write(evlist[j].data.fd, buf, MAXBUF);
                if (s == -1)
                    eperror("read");
                //printf("   sent back\n");
            }
            else if(evlist[j].events & (EPOLLHUP | EPOLLERR)){
                printf("    closing fd %d \n", evlist[j].data.fd);
                if(close(evlist[j].data.fd) == -1)
                    eperror("close");
                //numOpenFDs--;
            }
        }
    }
}
