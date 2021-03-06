/*
Copyright (c) 2008, Max Vilimpoc, http://vilimpoc.org/

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

    * Redistributions of source code must retain the above
      copyright notice, this list of conditions and the following
      disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials
      provided with the distribution.
    * Neither the name of the author nor the names of its
      contributors may be used to endorse or promote products
      derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <getopt.h>

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <pthread.h>

typedef enum
{
    DESTINATION = 0x0001,
    DEF_PORT    = 1025,
    MIN_REPEAT  = 20,
    PACKET_SIZE = 32,
} CONSTS;

static bool keepRunning = true;
float nextTime(float rateParameter);

static void Usage(const char * const progName)
{
    /* Here are the valid commands. */
    printf("\n");
    printf("%s accepts the following parameters: \n\n", progName);
    printf("\t* --destination or -d [ip address]\n");
    printf("\t  --port        or -p [port number]          (default: 1025)\n");
    printf("\t  --repeat      or -r [repeat every x msec.] (default: 20)\n");
    printf("\n");
}

void sigintHandler(int signal)
{
    keepRunning = false;
}

void *listen_handler(void *args){
    int* destSocket = (int *)args;
    int n;
    long int diff, temp;
    char timestamp[150];
    struct timeval beforeSleep, afterSleep, rec_time, send_time;
    double d1,d2, d3;
    char iust[5] = "IUST:";
    while(1){
        n = recv(*destSocket, timestamp, 150, 0 );
        if( n > 1){
          gettimeofday(&afterSleep, 0);
          //printf("after: %.10f \n", d2);
          //printf("erfan\t %d\n", (int) (d2 - d1) );
          //if(memcmp(timestamp, iust, 5) == 0){
              memcpy(&rec_time, &timestamp[5], sizeof(rec_time));
              memcpy(&send_time, &timestamp[21], sizeof(send_time));
	      d2 = atof(timestamp+37);
	      diff = ((send_time.tv_sec - rec_time.tv_sec)*1000000.0 + send_time.tv_usec - rec_time.tv_usec);
	      d1 = afterSleep.tv_sec* 1000000.0 + afterSleep.tv_usec;
              d3 = d1 - d2;
              fprintf(stderr, "%f\t %ld\n", d3, diff);
              //clear the message buffer
              memset(timestamp, 0, 150);
          //}
        }
    }
    return 0;

}

int main(int argc, char **argv)
{
    float speed;
    int theOption, theOptionIndex;
    pthread_t listener;
    const char *short_options = "d:p:r:";
    static struct option long_options[] =
    {
        { "destination", required_argument, NULL, 'd' },
        { "port",        required_argument, NULL, 'p' },
        { "repeat",      required_argument, NULL, 'r' },
        { 0, 0, 0, 0 }
    };

    int requiredParameters = DESTINATION;

    struct in_addr destAddr;
    memset(&destAddr, 0, sizeof(struct in_addr));

    uint16_t destPort = DEF_PORT;
    uint32_t repeatMs = MIN_REPEAT;

    while (-1 != (theOption = getopt_long(argc, argv, short_options, long_options,
                                          &theOptionIndex)))
    {
        switch(theOption)
        {
            case 'd':
                inet_pton(AF_INET, optarg, &destAddr);
                requiredParameters &= ~DESTINATION;
                break;
            case 'p':
                destPort = (uint16_t) atoi(optarg);
                break;
            case 'r':
                {
                speed = (float) atof(optarg);
                }
                break;
            case 'h':
            case '?':
            default:
                Usage(argv[0]);
                return -1;
                break;
        }
    }
    /* If not all required parameters were entered, then we need to exit. */
    if (0 != requiredParameters)
    {
        printf("Not all * required parameters were entered.\n");
        Usage(argv[0]);
        return -1;
    }
    /* Setup signal handler. */
    signal(SIGINT, sigintHandler);

    /* Data packet to send. */
    char dataPacket[150] = "IUST:000000000000000000000000000000000000000000000000";
    //memset(dataPacket[37], '0', 2413);

    /* Initialize the random number generator. */
    srand(clock());

    /* Destination ok. */
    int destSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (destSocket < 0)
    {
        printf("destSocket not opened.\n");
        return -1;
    }

    struct sockaddr_in destSocketAddr;
    destSocketAddr.sin_family   = AF_INET;
    destSocketAddr.sin_addr     = destAddr;
    destSocketAddr.sin_port     = htons(destPort);
    int i = 1;
    setsockopt( destSocket, IPPROTO_TCP, TCP_NODELAY, (void *)&i, sizeof(i));
    char arr[24];
    struct timeval afterSleep;
    float now;
    double d1;
        /* Now connect to the server */
    if (connect(destSocket, (struct sockaddr*)&destSocketAddr, sizeof(destSocketAddr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }
    int pt = pthread_create(&listener, NULL, listen_handler, (void *) &destSocket);
    if (pt < 0){
        perror("pthread create failed!");
    }
    while(keepRunning){
      now = (nextTime(speed) * 1000000);
      usleep(now);
      gettimeofday(&afterSleep, 0);
      d1 = afterSleep.tv_sec*1000000.0 +  afterSleep.tv_usec;
      sprintf(arr,"%f", d1);
      //printf("arr %s, %d, %d\n", arr, sizeof(arr), sizeof(double));
      strcpy(dataPacket + 37, arr);
      //memcpy(&dataPacket + 37, &afterSleep, sizeof(afterSleep));
      //printf("packet: %s\n", dataPacket);
      if (0 > sendto(destSocket,
                     &dataPacket,
                     sizeof(dataPacket),
                     0,
                     (struct sockaddr *) &destSocketAddr,
                     sizeof(destSocketAddr)))
      {
          perror("sendto");
      }
    }

    /* Cleanup. */
    //printf("Shutting %s down.\n", argv[0]);
    close(destSocket);

    return 0;
}

float nextTime(float rateParameter)
{
  float temp = -logf(1.0f - (float) random() / (RAND_MAX)) / rateParameter;
  //printf("next: %f\n",temp);
    return temp;
}
