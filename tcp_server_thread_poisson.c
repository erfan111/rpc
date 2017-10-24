/*
    C socket server example, handles multiple clients using threads
    Compile
    gcc server.c -lpthread -o server
*/

#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
#include <math.h>

struct thread_param {
    int socket_desc;
    int lambda;
};

float nextTime(float rateParameter);

//the thread function
void *connection_handler(void *);

int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c, lambda = 1;
    struct sockaddr_in server , client;

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
    if(argc > 1)
        lambda = atoi(argv[1]);

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


    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
	pthread_t thread_id;
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        struct thread_param tp;
        puts("Connection accepted");
        tp.socket_desc = client_sock;
        tp.lambda = lambda;
        if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &tp) < 0)
        {
            perror("could not create thread");
            return 1;
        }

        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( thread_id , NULL);
        puts("Handler assigned");
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
    int lambda = tp->lambda;
    int read_size;
    char *message , client_message[2000];
    float now;

    //Receive a message from client
    while( (read_size = recv(sock , client_message , 37 , 0)) > 0 )
    {
        //end of string marker
	//	client_message[read_size] = '\0';
	struct timeval my_time;
	char iust[5] = "IUST:";
    now = (nextTime(lambda));
    usleep(now);
	if(memcmp(client_message, iust, 5) == 0){
    	memcpy(&my_time, &client_message[5], sizeof(my_time));
    		//Send the message back to client
            write(sock , client_message , 37);

    		//clear the message buffer
    		memset(client_message, 0, 37);
    	}
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


float nextTime(float rateParameter)
{
    float temp = -logf(1.0f - (float) random() / (RAND_MAX)) / rateParameter;
    return temp;
}
