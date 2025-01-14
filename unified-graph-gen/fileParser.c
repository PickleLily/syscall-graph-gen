#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

int root_node_id = -1;
#define MAX_NODES 1000

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

// Helper function to parse file descriptors with "<f>"
void parse_file_descriptor(const char *fd, char *file_path){
        char *start = strstr(fd, "<f>");
        if (start) {
                start += 3; // Skip past "<f>"
                char *end = strchr(start, ')');
                if (end) {
                    size_t length = end - start;
                    strncpy(file_path, start, length);
                    file_path[length] = '\0'; // Null-terminate the extracted path
                 } else {
                        strncpy(file_path, "Unknown File", 255);
                 }
        } else {
                strncpy(file_path, fd, 255); // If no "<f>", use the entire fd string
        }
}

Inputs* parseInput(){
        FILE *input;
	char line[1024];
	int node = 0;
        Inputs *il = malloc(sizeof(Inputs) * 1000);

	input = fopen("input.txt", "r");
         if(!input){
                perror("Failed to open the input file...\n");
        }

        while(fgets(line, sizeof(line), input)){
		char t[64], type[64], name[64], syscall[64], args[1024];
		sscanf(line, "%s %s Name=%s Syscall=%s Args=%[^\n]", t, type, name, syscall, args);
		Inputs i = makeInput(t, type, name, syscall, args);

		il[node] = i;
		node = node + 1;
		//printf("%s\n", i->syscall_name);

                //extract root node from first accept4 syscall
                if (root_node_id == -1 && strcmp(syscall, "accept4") == 0) {
                    char fd[256] = "Unknown FD";
                    if (strstr(args, "fd=")) {
                        sscanf(args, "fd=%255[^ ]", fd); // Extract fd field
                        parse_file_descriptor(fd, fd);  // Extract file path from fd
                    }
               }
        }
        return il;
}


int main(){
	Inputs *inputs = parseInput();
	printf("inputs: %s\n", inputs);
}
