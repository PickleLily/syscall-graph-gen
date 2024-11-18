#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>

//network imports
#include <sys/types.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#include <regex.h>

#define SIZE 1024

void DieWithError(char *errorMessage)
{
	printf(errorMessage + "\n");
    exit(0);
} 

void HandleTCPClient(int clntSocket) /* TCP client handling function */
{
	char buf[SIZE];
	int read = recv(clntSocket, buf, SIZE, 0);
      	if (read < 0) DieWithError("Client read failed\n");
	
	printf("Recieved: %s", buf);
}



int main(int argc, char *argv[]){

    // int printRTT = 0; //0 == store --> 1 == print
    char buffer[SIZE]; //for the request / resp

    int sock;
    struct sockaddr_in serverAddress;
    int serverPort;
    struct hostent *server;

    
    char* serverIP;
    char host[100];
    char path[100];
    for(int i = 0; i < 100; i++){
        host[i] = 0;
        path[i] = 0;
    }

    char message[4096];
    char *response[4096];

    memset(response,0,sizeof(response));
    int totalSize = sizeof(response)-1;
    int received = 0;
    int bytes = 0;

    struct timeval time0, time1;


//0 read in args from cmd line and set appropriate values

    if((argc < 3) || (argc > 4)){ 
        fprintf(stderr, "Usage: %s [-options] server_url port_number\n", argv[0]);
        exit(1);
    }
        if(argc == 4){
		if(strcmp(argv[1], "-p") != 0){
			printf("The only optional argument is -p.");
			exit(1);
		}else{
            //printf("Print option chosen. \n");
			printRTT = 1;
		}
    }

    serverIP = argv[argc - 2];
    serverPort = atoi(argv[argc - 1]); 

    char split[] = "/";
    int pos = strcspn(serverIP, split);
    printf("Len is %i\n", pos);

    
    if(pos == strlen(serverIP)){
        strncpy(host, serverIP, strlen(serverIP));
        strncpy(path, "/", strlen("/"));
    }else{
        strncpy(host, serverIP, pos);
        strncpy(path, serverIP + (pos), strlen(serverIP));
    }

    printf("Port:%d\n", serverPort);
    printf("host: %s\n", host);
    printf("path: %s\n", path);


//--------------------------------------1 - make the socket--------------------------------------

    if ((sock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        DieWithError("socket() failed");
    }

    printf("%s\n", host);
    printf("Attempting to connect on host %s port %i\n", host, serverPort);
    server = gethostbyname(host);
   

    // Get server information
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(host, argv[argc-1], &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(1);
    }
    printf("Got host!\n");
    fflush(stdout);

    //use the arpa/inet to create the server address struct...
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);

    memcpy(&serverAddress.sin_addr.s_addr, server->h_addr, server->h_length);


    //--------------------------------------Step 2 - connect--------------------------------------


   if(gettimeofday(&time0, NULL) == -1){
        DieWithError("gettimeofday fail???");
   }

    int result = connect (sock, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    if ( result < 0){
            DieWithError("connect() failed\n");
        }   

    printf("Connected!\n");
    fflush(stdout);

    if(gettimeofday(&time1, NULL) == -1){
        DieWithError("gettimeofday fail???\n");
   }
    

    long RTT = (time1.tv_sec - time0.tv_sec) * 1000 + (time1.tv_usec - time0.tv_usec) / 1000; 

 
//--------------------------------------Step 3 - send HTTP request--------------------------------------


    sprintf(message,"GET %s HTTP/1.1\r\n\r\n", path, host);
    printf("Request:\n%s\n",message);

    if (send (sock, message, strlen(message), 0) != strlen(message)){
       DieWithError("send() sent a different number of bytes than expected\n");
    }else{
        printf("Socket Connected!\n");
    }

    printf("Sent!\n");
    fflush(stdout);

//--------------------------------------Step 4 - recieve a http response--------------------------------------
    FILE* f;

    do{
        bytes = read(sock, response + received, totalSize - received);
        if (bytes < 0)
            //DieWithError("ERROR reading response from socket");
        if (bytes == 0)
            break;
        received += bytes;
    } while (received < totalSize);
printf("recieved message\n");
    close(sock);

    printf("Read!\n");
    fflush(stdout);


//5 - RTT and either print or save a file depending on the optional value...
    f = fopen("/home/lkbromberger/Networks-1/project1/client/RTT.txt", "w");
    if(f == NULL){
        printf("There was an issue opening the file...");
        exit(1);
    }else{
        printf("File Opened\n");
    }

    fprintf(f,"HTTP Response:\n%s\n\n",response);
    fprintf(f, "Round-trip time: %ld ms\n", RTT);
    printf("Page saved to RTT.txt");
    fclose(f);

    printf("Server Response:\n--------------------\n%s\n\n",response);

    if(printRTT != 0){  // display RTT on screen
        printf("Round-trip time: %ld ms\n", RTT);
    }

    exit(0);
}