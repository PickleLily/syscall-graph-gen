#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
//#include "syscalls.h"


#define MAX_LINE_LENGTH 200
#define PRIORITY "Debug"
#define MAX_NODES 2000
#define MAX_EDGES 1000


typedef struct Node {
    char syscall[64];
    int id;         // Unique identifier
} node_t;

typedef struct Edge {
    int id;
    int from;       // ID of the source node
    int to;         // ID of the destination node
} edge_t;


// Global graph representation
int node_count = 0, edge_count = 0;
node_t nodes[MAX_NODES];
edge_t edges[MAX_EDGES];
int root_node_id = -1;


//goes through ALL nodes and either returns the index that the syscall is at in our list of nodes 
//OR
//adds the system call as a unique Node
int find_or_add_node(const char *syscall_name) {
	//for each node in the list of nodes, if the name exists, return the index ELSE constinue
	for (int i = 0; i < node_count; i++) {
        	if (strcmp(nodes[i].syscall, syscall_name) == 0) {
            	return nodes[i].id;
        	}
    	}
	//if the node does NOT exist, & we exceed the max #of nodes crash...
   	if (node_count >= MAX_NODES) {
        	fprintf(stderr, "Error: Maximum nodes exceeded.\n");
        	exit(1);
    	}

	//if the node does NOT exist and we have NOT exceeded the max # create a new node And write the name and return the node ID
   	nodes[node_count].id = node_count;
    	strncpy(nodes[node_count].syscall, syscall_name, sizeof(nodes[node_count].syscall) - 1);
    	return nodes[node_count++].id;
}

//takes the ints representing two syscalls and sees if there is an edge connecting the two
int find_or_add_edge(int prev, int current){
	//go through all of the existing edges
	for(int i = 0; i < edge_count; i ++){
		//if there is a edge with the same 
		if(prev == 0 && current == 0){
			return -1;
		}

		if(edges[i].from == prev && edges[i].to == current){
			//it IS not recording every edge... but why are SOME being repeated...
//			printf("repeat.. disregarded %d %d \n", edges[i].from, edges[i].to);
			return edges[i].id;
		}
	}

	//too many edges
	if(edge_count >= MAX_EDGES){
		fprintf(stderr, "Error: Maximum number of nodes exceeded \n");
		exit(1);
	}

	//how is this returning anything that is not just 1???
	edges[edge_count].id = edge_count;
	edges[edge_count].from = prev;
	edges[edge_count].to = current;
	return edges[edge_count++].id;
}

int main(int argc, char* argv[]){
	if(argc != 2){
		perror("Error: You must specify the file that you want to parse...\n");
		return -1;
	}

	//try to open the input/output file
	FILE *f = fopen(argv[1], "r");
	FILE *fout = fopen("output.txt", "w");
        if(f == NULL || fout == NULL){
                perror("Error: opening file...\n");
                return -1;
        }


	char currentLine[MAX_LINE_LENGTH]; //input line
	char *token; // the part of the line we are reading
	char *syscall = malloc(MAX_LINE_LENGTH);
	char *pastCall = malloc(MAX_LINE_LENGTH);
	int pastCall_num;
	node_t all_nodes[MAX_NODES];

	//while NOT at the end of the file...
	while(fgets(currentLine, MAX_LINE_LENGTH-1, f) != NULL){
		//if the line does NOT have the priority ... get the next line...
		if(strstr(currentLine, PRIORITY) == NULL){
			break;
			printf("parsing error line... \n");
		}

		//returns time
		strtok(currentLine, " ");
		//token == PRIORITY
		token = strtok(NULL, " ");
		//token == proc.name
		token = strtok(NULL, " ");
		//get the syscall
		syscall = strtok(NULL, " \n");

		int syscall_num = 0;
		//IFF Not the first node, add an edge!
		if(pastCall != NULL){

			//if the syscall is not a repeat, log it to ourput file and track edge.
			if(strcmp(pastCall, syscall) != 0){
				syscall_num = find_or_add_node(syscall);

				fprintf(fout, "%s\n",syscall);
				find_or_add_edge(pastCall_num, syscall_num);
//				printf("%s %s\n", syscall, pastCall);
			}else{
				syscall_num = find_or_add_node(syscall);
			}
		}
		strcpy(pastCall, syscall);
		pastCall_num = syscall_num;

	}

	// should probably free the syscall and pastcall here...
	fclose(f);
	fclose(fout);

	//Output for the graph generation... (graph.dot)
	FILE *dot_file = fopen("type1_graph.dot", "w");
	if(!dot_file){
		perror("failed to open .DOT file\n");
	}

	fprintf(dot_file, "digraph nginx_syscalls{\n");
	for(int i = 0; i < node_count; i++){
		fprintf(dot_file, " %d [label=\"%s\"];\n", nodes[i].id, nodes[i].syscall);
	}

	for(int i = 0; i < edge_count; i++){
		fprintf(dot_file, " %d -> %d\n", edges[i].from, edges[i].to);
	}

		fprintf(dot_file, "}");

	fclose(dot_file);
	printf("Graph exported to type1_graph.dot\n");

	return 0;
}

