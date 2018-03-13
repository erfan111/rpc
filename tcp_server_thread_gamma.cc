/*
    C socket server example, handles multiple clients using threads
    Compile
    gcc server.c -lpthread -o server
*/

#include <stdio.h>
#include <string.h>    //strlen
#include <stdlib.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <pthread.h> //for threading , link with lpthread
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include <random>

struct thread_param {
    int socket_desc;
    int k;
    int theta;
};

std::default_random_engine generator;


//the thread function
void *connection_handler(void *);

int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c, k = 1, theta = 1;
    struct sockaddr_in server , client;
    srand(time(NULL));
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Gamma Socket created");
    if(argc == 3){
        k = atoi(argv[1]);
        theta = atoi(argv[2]);
    }
    else{
        fprintf(stderr, "provide k and theta \n");
        exit(1);
    }

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    //Listen
    listen(socket_desc , 1000);


    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
	pthread_t thread_id;
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        struct thread_param tp;
        //puts("Connection accepted");
        tp.socket_desc = client_sock;
        tp.k = k;
        tp.theta = theta;
        if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &tp) < 0)
        {
            perror("could not create thread");
            return 1;
        }

        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( thread_id , NULL);
        //puts("Handler assigned");
    }

    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }

    return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *param)
{
    //Get the socket descriptor
    struct thread_param *tp = (struct thread_param*) param;
    int sock = tp->socket_desc;
    int k = tp->k;
    int theta = tp->theta;
    int read_size;
    char client_message[150];
    double now;
	double difference;
    std::gamma_distribution<double> distribution(k, theta);
    //Receive a message from client
    while( (read_size = recv(sock , client_message , 37 , 0)) > 0 )
    {
        //end of string marker
	//	client_message[read_size] = '\0';
	struct timeval my_time, moment;
	do {now = distribution(generator);}
    while (now < 0);
	printf("now = %f\n");
	gettimeofday(&my_time, 0);
	//printf("it will take %f us\n", now);
	do {
		gettimeofday(&moment, 0);
		difference = moment.tv_usec - my_time.tv_usec;
	} while(difference < now);
    write(sock , client_message , 37);

	//clear the message buffer
	memset(client_message, 0, 37);
    }

    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }

    return 0;
}

float nextTime(float mean, float stddev)
{//Box muller method
    static float n2 = 0.0;
    static int n2_cached = 0;
    if (!n2_cached)
    {
        float x, y, r;
        do
        {
            x = 2.0*rand()/RAND_MAX - 1;
            y = 2.0*rand()/RAND_MAX - 1;

            r = x*x + y*y;
        }
        while (r == 0.0 || r > 1.0);
        {
            float d = sqrt(-2.0*log(r)/r);
            float n1 = x*d;
            n2 = y*d;
            float result = n1*stddev + mean;
            n2_cached = 1;
            return result;
        }
    }
    else
    {
        n2_cached = 0;
        return n2*stddev + mean;
    }
}
