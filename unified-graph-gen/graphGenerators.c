#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "graphMaker.h"

#define MAX_EDGES 1000
#define MAX_NODES 1000

#define MAX_SYSCALLS 373

//SETUP AREA

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


//this graph generation with nodes and edges is made from Lilys version of graph generation...
void dotFileGeneration(FILE *file, int node_count, int edge_count, int nodes, int edges){
	fprintf(file, "digraph nginx_syscalls {/n");
	for(int i = 0; i < node_count; i++){
		fprintf(file, "  %d [label=\"%s\"];\n", nodes[i].id, nodes[i].name);
	}

	for(int i = 0; i < edge_count; i++){
		fprintf(file, "  %d -> %d [label = \"%s\"];\n]", 
			edges[i].parent, edges[i].child, edges[i].syscall);
	}

	fprintf(file, "}\n");
	return;
}


//Graph 1 Generation
/**/
void graph1_generation(){
	return;
}


//Graph 2 Generation
/**/
void graph2_generation(){
	return;
}


//Graph 3 Generation
/**/
void graph3_generation(){
	return;
}


//Generate all three graphs

int main(){
	printf("start.../n");

//open all the input and the output files for the graph generation...
	FILE *input;
	FILE *type1_output;
	FILE *type2a_output;
	FILE *type2b_output;

	input = fopen("graphInput.txt", "r");
	//output is .dot file for the graph gen library...
	type1_output = fopen("graph1.dot", "w");
	type2a_output = fopen("graph2a.dot", "w");
	type2b_output = fopen("graph2b.dot", "w");

	if(!type1_output || !type2a_output || !type2b_output){
		perror("Failed to open an output file.../n");
		return -1;
	}
	if(!input){
		perror("Failed to open the input file.../n");
	}


	printf("Generating Type 1a Graph/n");
	graph1_generation();

	printf("Generating Type 2 Graph/n");
	graph2_generation();

	printf("Generating Type 2b Graph/n");
 	graph3_generation();

	printf("fin.");

	//cleanup
	fclose(input);
	fclose(type1_output);
	fclose(type2a_output);
	fclose(type2b_output);

}
