#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

int main(int argc, char* argv[]){
    if (argc < 2){
        printf("did not specify enough arguments...\n");
        return -1;
    }
    int arg = atoi(argv[1]);
    
    if(arg == 1){
        printf("specified option 1");
    }

    else if(arg == 2){
        printf("specified option 2");
        //send http request to https://icanhazdadjoke.com/api

        //create socket to send http message
        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        connect(sock, (struct sockaddr *) &serverAddress, sizeof(serverAddress));

        //load this custom string into the message variable
        char message[1024];
        char path[1024] = 'https://icanhazdadjoke.com/j/';

        //generate random number
        srand(42);
        int random_number = (rand() % 100);
        char random_number_string[10];
        sprintf(random_number_string, "%d", random_number);

        //generate path with random joke ID number
        strcat(path, random_number_string);
        
        //create HTTP req.
        sprintf(message, "GET %s HTTP/1.1\r\n\r\n", path);

        //send HTTP message
        send(sock, message, strlen(message), 0);

    }
    else if(arg == 3){
        time_t t;
        time(&t);
        printf("current time:%s", ctime(&t));
        return 0;  
    }else{
        printf("Specified an option that was not available");
        return -1;
    }  
}