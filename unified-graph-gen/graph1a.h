#include <stdio.h>
// #include <stdbool.h>
#include <string.h>
#include <stdlib.h>

/* */

#define NUM_SYSCALLS 373
#define MAX_NODES 1000
#define MAX_EDGES 2000

// Structures for Nodes and Edges
typedef struct Node {
    char syscallName[64]; 
    char args[256]; 
    int id;
} Node;

typedef struct Edge {
    int parent;       // ID of the source node
    int child;        // ID of the destination node
    int sysCallNum;
    char syscall[64]; // The system call connecting the nodes
} Edge;

// Global graph representation
Node nodes[MAX_NODES];
Edge edges[MAX_EDGES];
int node_count = 0, edge_count = 0;

// // Global root node ID
// int root_node_id = -1;

// Function to find or add a node
int find_or_add_node(const char *name) {
    for (int i = 0; i < node_count; i++) {
        if (strcmp(nodes[i].syscallName, name) == 0) {
            return nodes[i].id;
        }
    }
    if (node_count >= MAX_NODES) {
        fprintf(stderr, "Error: Maximum nodes exceeded.\n");
        exit(1);
    }
    nodes[node_count].id = node_count;
    strncpy(nodes[node_count].syscallName, name, sizeof(nodes[node_count].syscallName) - 1);
    return nodes[node_count++].id;
}

// Function to add an edge
void add_edge(int from, int to, const char *syscall) {
    if (edge_count >= MAX_EDGES) {
        fprintf(stderr, "Error: Maximum edges exceeded.\n");
        exit(1);
    }
    edges[edge_count].parent = from;
    edges[edge_count].child = to;
    strncpy(edges[edge_count].syscall, syscall, sizeof(edges[edge_count].syscall) - 1);
    edge_count++;
}

// // Helper function to parse file descriptors with "<f>"
// void parse_file_descriptor(const char *fd, char *file_path) {
//     char *start = strstr(fd, "<f>");
//     if (start) {
//         start += 3; // Skip past "<f>"
//         char *end = strchr(start, ')');
//         if (end) {
//             size_t length = end - start;
//             strncpy(file_path, start, length);
//             file_path[length] = '\0'; // Null-terminate the extracted path
//         } else {
//             strncpy(file_path, "Unknown File", 255);
//         }
//     } else {
//         strncpy(file_path, fd, 255); // If no "<f>", use the entire fd string
//     }
// }

// Main function to parse Falco output and build the graph
int main() {
    FILE *file = fopen("graph1a_out.txt", "r");
    if (!file) {
        perror("Failed to open events file");
        return 1;
    }

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        char syscall[64], args[512];
        if (sscanf(line, "Syscall=%s Args=%[^\n]", syscall, args) != 2) {
            printf("%s %s \n",syscall, args);
            continue;
        }

        // Extract root node from the first accept4 syscall
        if (root_node_id == -1 && strcmp(syscall, "accept4") == 0) {
            char fd[256] = "Unknown FD";
            if (strstr(args, "fd=")) {
                sscanf(args, "fd=%255[^ ]", fd); // Extract fd field
                parse_file_descriptor(fd, fd);  // Extract file path from fd
            }
            root_node_id = find_or_add_node(fd); // Use fd as root node
            continue;
        }

        // Parse arguments for objects like files or sockets
        char object1[256] = "";
        if (strstr(args, "path=")) {
            sscanf(args, "path=%255s", object1); // Extract file path
        } else if (strstr(args, "fd=")) {
            char fd[256];
            sscanf(args, "fd=%255[^ ]", fd); // Extract fd field
            if (strstr(fd, "<f>")) {
                parse_file_descriptor(fd, object1); // Extract file path if "<f>"
            } else {
                strncpy(object1, fd, 255); // Use fd as-is if not a file
            }
        }

        // Add edges if relevant objects are found
        if (strlen(object1) > 0 && root_node_id != -1) {
            int to = find_or_add_node(object1);
            add_edge(root_node_id, to, syscall);
        }
    }

    fclose(file);

    // Output the graph in DOT format
    FILE *dot_file = fopen("graph.dot", "w");
    if (!dot_file) {
        perror("Failed to open DOT file");
        return 1;
    }

    fprintf(dot_file, "digraph nginx_syscalls {\n");
    for (int i = 0; i < node_count; i++) {
        fprintf(dot_file, "  %d [label=\"%s\"];\n", nodes[i].id, nodes[i].syscallName);
    }
    for (int i = 0; i < edge_count; i++) {
        fprintf(dot_file, "  %d -> %d [label=\"%s\"];\n",
                edges[i].parent, edges[i].child, edges[i].syscall);
    }
    fprintf(dot_file, "}\n");

    fclose(dot_file);
    printf("Graph exported to graph.dot\n");
    return 0;
}



void generate_1a(){
    printf("generated graph 1a\n");
    m();
}