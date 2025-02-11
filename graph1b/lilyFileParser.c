#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

int root_node_id = -1;
#define MAX_NODES 1000

//input holds all of the information needed for all three graphs generations...
typedef struct input{
	char time[64];
	char type[64];
	char process_name[64];
	char syscall_name[64];
	char args[1024];
	int inputID;
}Inputs;

Inputs makeInput(char time[], char type[], char process[], char syscall[], char args[]){
	Inputs newInput;
	strcpy(newInput.time, time);
	strcpy(newInput.type, type);
	strcpy(newInput.process_name, process);
	strcpy(newInput.syscall_name, syscall);
	strcpy(newInput.args, args);
	return newInput;
}

Inputs* parseInput(int *inputCount){
    FILE *input;
	char line[1024];
	int node = 0;
    Inputs *il = malloc(sizeof(Inputs) * MAX_NODES);


	//TODO -- make this the falco output path
	input = fopen("input.txt", "r");
    if(!input){
        perror("Failed to open the input file...\n");
		free(il);
    }

	while(fgets(line, sizeof(line), input)){
		char t[64], type[64], name[64], syscall[64], args[1024];
		sscanf(line, "%s %s Name=%s Syscall=%s Args=%[^\n]", t, type, name, syscall, args);
		Inputs i = makeInput(t, type, name, syscall, args);

		il[node] = i;
		node++;

		//extract root node from first accept4 syscall
		if (root_node_id == -1 && strcmp(syscall, "accept4") == 0) {
			char fd[256] = "Unknown FD";
			if (strstr(args, "fd=")) {
				sscanf(args, "fd=%255[^ ]", fd); // Extract fd field
				// parse_file_descriptor(fd, fd);  // Extract file path from fd
			}
		}
	}
	*inputCount = node;
    return il;
}

int main(){
	//this is returning just the first system call NOT a list of them all...
	int inputCount = 0;
	Inputs *inputs = parseInput(&inputCount);

    FILE *intermediateOutput = fopen("intermediateOutput.txt", "w");

	int i = 0;
	for(int i = 0; i < inputCount; i++){
		fprintf(intermediateOutput, "%s\n", inputs[i].syscall_name);
	}
}
