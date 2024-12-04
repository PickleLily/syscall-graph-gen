#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LINE_LENGTH 200
#define PRIORITY "Debug"
#define MAX_NUM_NODES 364

typedef struct node{
	int id;
	char *syscall_name;
	int connections[];
}t_node;

int main(int argc, char* argv[]){
	if(argc != 2){
		printf("You must specify the file that you want to parse...\n");
		return -1;
	}

	//try to open the input/output file
	FILE *f = fopen(argv[1], "r");
	FILE *fout = fopen("output.txt", "w");
        if(f == NULL || fout == NULL){
                perror("Error opening file");
                return -1;
        }


	char currentLine[MAX_LINE_LENGTH]; //input line
	char *token; // the part of the line we are reading
	char *syscall = malloc(MAX_LINE_LENGTH); 
	char *pastCall = malloc(MAX_LINE_LENGTH);

	node_t all_nodes[MAX_NUM_NODES];
	int count = 0;

	//while NOT at the end of the file...
	while(fgets(currentLine, MAX_LINE_LENGTH-1, f) != NULL){
		//if the line does NOT have the priority ... get the next line...
		if(strstr(currentLine, PRIORITY) == NULL){
			break;
			printf("line has no debug");
		}

		//returns time
		strtok(currentLine, " ");
		//token == PRIORITY
		token = strtok(NULL, " ");
		//token == proc.name
		token = strtok(NULL, " ");
		//get the syscall
		syscall = strtok(NULL, " ");


		//Only record if it is a unique call...
		if(pastCall != NULL){

			printf("last: %s now: %s \n", pastCall, syscall);

			//if the system call is NOT a repeat
			if(strcmp(pastCall, syscall) != 0){
				bool found = false;
				//look to see if a Node for the syscall exists
				while(!found){
					node_t existing_node = all_nodes[count];
					//if it exists
					if(strcmp(existing_node.syscall_name, syscall) == 0 ){
						//does the syscall have a connection?
						//if no make a new connection
						//stop looking for the node
						found = true;
					}
					count ++;
				}
				node_t n;
				n.id = count;
				n.syscall_name = pastCall;
				n.connections[connections_len] = id
				fprintf(fout, syscall);

			}
		}
		strcpy(pastCall, syscall);
	}
	free(pastCall);
	free(syscall);
	fclose(f);
	fclose(fout);
	printf("files closed\n");
	return 0;
}

