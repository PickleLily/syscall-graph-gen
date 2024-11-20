#include <stdio.h>
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h> 

#define MAXPENDING 5 /* Maximum outstanding connection requests */
#define SIZE 1024
void DieWithError(char *errorMessage); /* Error handling function */
void HandleTCPClient(int clntSocket); /* TCP client handling function */



//Start server & wait for connection
void CreateHttpResponse();
void HandleTCPClient(int clntSocket);


int main(int argc, char *argv[])
{	
	int servSock; 
	int clntSock; 
	struct sockaddr_in ServAddr;
	struct sockaddr_in ClntAddr;
	unsigned short ServPort;
	unsigned int clntLen;

	
	if (argc != 2){
		fprintf(stderr, "Usage: %s <Server Port>\n", argv[0]);
		exit(1);
	}

	ServPort = atoi(argv[1]);
	printf("server Port #: %d\n", ServPort);

	//printf("port # %d\n", ServPort);

	if ((servSock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
		DieWithError("socket() failed\n"); 
	}
	printf("socket created\n");

	memset(&ServAddr, 0, sizeof(ServAddr));
	ServAddr.sin_family = AF_INET;
	ServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	ServAddr.sin_port = htons(ServPort);

	if (bind (servSock, (struct sockaddr *) &ServAddr, sizeof(ServAddr))<0){
		DieWithError("bind() failed\n");
	}
	printf("bound\n");
	
	if (listen (servSock, MAXPENDING) < 0){
		DieWithError("listen() failed");
	}
	printf("listening...\n");
	


	//run forever
	while(1){
		clntLen = sizeof(ClntAddr);
		if ((clntSock = accept (servSock, (struct sockaddr *) &ClntAddr, &clntLen)) < 0){
			DieWithError("accept() failed");
		}
		printf("Handling client %s\n", inet_ntoa(ClntAddr.sin_addr));
		HandleTCPClient(clntSock);

		CreateHttpResponse();
	}
}


void DieWithError(char *errorMessage){
	printf(errorMessage);
	exit(0);
}



void HandleTCPClient(int clntSocket) /* TCP client handling function */
{
	char buf[SIZE];
	int read = recv(clntSocket, buf, SIZE, 0);
    if (read < 0){
		 DieWithError("Client read failed\n");
	}

	printf("Recieved: %s", buf);

	char method[16], path[1024], http[16];

	sscanf(buf, "%s /%s %s", method, path, http);
	printf("%s /%s %s", method, path, http);

    // Making sure that the request method is a get
    if (strcmp(method, "GET") != 0)
    {
    	DieWithError("Method Not Allowed\n");
	}else{
		FILE *f; 
		//is the file right?
		if (strcmp(path, "index.html") || path == NULL || strcmp(path, "TDMG.html")){

			f = fopen("TMDG.html", "r");

			if(f != NULL){
				printf("file is opened\n");
			}else{
				DieWithError("File opening Error");
			}
			

			fseek(f, 0, SEEK_END);
            long fileSize = ftell(f);
            fseek(f, 0, SEEK_SET);

			char *fileContent = malloc(fileSize + 1);
            fread(fileContent, fileSize, 1, f);
			
            fclose(f);

            fileContent[fileSize] = 0;

            if (write(clntSocket, fileContent, fileSize) < 0)                     // Send the content back to the host.
            {
                DieWithError("Error: Could not write to socket");
            }
            free(fileContent);
		}else{
			printf("here");
    		DieWithError("File not Found.\n");
		}
	}
}




void CreateHttpResponse(){
	char message[4096];
	sprintf(message,"HTTP/1.1 200 OK \r\nContent-Type: text/html \r\n\r\n");
}