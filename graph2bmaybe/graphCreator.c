#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NODES 1000
#define MAX_EDGES 2000

// Structures for Nodes and Edges
typedef struct Node {
    char PID[64]; // PID
    char args[1024]; // Name of the object (e.g., file path, socket info)
    int id; // Unique identifier
    int isExt;
} Node;

typedef struct Edge {
    int from;       // ID of the source node
    int to;         // ID of the destination node
    char syscall[64]; // The system call connecting the nodes
} Edge;

// Global graph representation
Node nodes[MAX_NODES];
Edge edges[MAX_EDGES];
int node_count = 0, edge_count = 0;

// Global root node ID
int root_node_id = -1;


// Function to find or add a node
int add_node(const char *args, char PID[], int node_count) {
    if (node_count >= MAX_NODES) {
        fprintf(stderr, "Error: Maximum nodes exceeded.\n");
        exit(1);
    }

    nodes[node_count].id = node_count;

    strncpy(nodes[node_count].args, args, sizeof(nodes[node_count].args) - 1);
    strncpy(nodes[node_count].PID, PID, sizeof(nodes[node_count].PID) - 1);

    return node_count + 1;
}

// Function to add an edge
void add_edge(int from, int to, const char *syscall) {
    for (int i = 0; i < edge_count; i++) {
        if (edges[i].from == from && edges[i].to == to &&
            strcmp(edges[i].syscall, syscall) == 0) {
            // Duplicate edge found, do not add
            return;
        }
    }	
    if (edge_count >= MAX_EDGES) {
        fprintf(stderr, "Error: Maximum edges exceeded.\n");
        exit(1);
    }
    edges[edge_count].from = from;
    edges[edge_count].to = to;
    strncpy(edges[edge_count].syscall, syscall, sizeof(edges[edge_count].syscall) - 1);
    edge_count++;
}

// Helper function to parse file descriptors with "<f>"
void parse_file_descriptor(const char *fd, char *file_path) {
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

// Main function to parse Falco output and build the graph
int main() {

    int numberOfNodes = 0;
    // open intermediate file
    FILE *file = fopen("intermediateOutput.txt", "r");
    if (!file) {
        perror("Failed to open events file");
        return 1;
    }else{
        printf("intermediateOutput.txt opened successfully...\n");
    }

    char line[1024];
    while (fgets(line, sizeof(line), file)) {

        char FD[1024]= "", syscall[1024] = "", args[1024] = "", ret[1024] = "", PID[1024] = "";
    
        int matches = sscanf(line, "%s %s %[^\n]", syscall, PID, args);
        // printf("%s %s %s\n", syscall, PID, args);

        if(strcmp(args, "Return") != 0){
            numberOfNodes = add_node(PID, args, numberOfNodes);
        }

        if(numberOfNodes > 0){
            add_edge(numberOfNodes-1, numberOfNodes, syscall);
        }

    }

    fclose(file);
    // Output the graph in DOT format
    FILE *dot_file = fopen("../graphs/graph2b.dot", "w");
    if (!dot_file) {
        perror("Failed to open DOT file");
        return 1;
    }else{
        printf("Successfully opened DOT file...\n");
    }

    fprintf(dot_file, "digraph nginx_syscalls {\n");

    printf("number of nodes: %d\n", numberOfNodes);

    for (int i = 0; i < numberOfNodes; i++) {
        fprintf(dot_file, "  %d [label=\"PID:%s %s\"];\n", nodes[i].id, nodes[i].args, nodes[i].PID);
    }

    for (int i = 0; i < edge_count; i++) {
        fprintf(dot_file, "  %d -> %d [label=\"%s\"];\n", edges[i].from, edges[i].to, edges[i].syscall);
    }

    fprintf(dot_file, "}\n");
    fclose(dot_file);
    printf("Graph exported to graphs/graph2.dot\n");
    return 0;
}