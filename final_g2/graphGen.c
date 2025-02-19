#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// --------------------SET UP---------------------------------------------------------
#define MAX_SUBNODES 100
#define MAX_SUBEDGES 1000
#define MAX_SUBGRAPHS 100

// Structures for Nodes and Edges
typedef struct Node {
    char PID[64]; // PID
    char name[256]; // Name of the object (e.g., file path, socket info, subprocesses info?)
    int fd; // The file descriptor
    int graphNum; //Unique subgraph
    char shape[128];
} Node;

typedef struct Edge {
    char from[256];       // name of the source node 
    char to[256];         // name of the destination node
    int graphNum; //Unique subgraph
    char syscall[64]; // The system call connecting the nodes
} Edge;

// Subgraph representation
typedef struct Subgraph {
    int graphNum; //Unique subgraph number
    int currentfd; //fd of the root accept4 node - not sure if we even need - shpuld this be current fd?
    Node* nodes[MAX_SUBNODES];
    Edge* edges[MAX_SUBEDGES];
    int node_count;
    int edge_count;
} Subgraph;

// Global graph ID
int graphNum = 0;

// Global graph Reference
Subgraph* graphs[MAX_SUBGRAPHS];

// ---------------------Functions --------------------------------------------------------


// When supplied with fd of accept4 call, make new split graph
void makeSubgraph(int fd, char *socketTuple) {
    // Take global graph num, current fd of accept4, and arg information to build the Subgraph
    // Predefine the two sockets


    // TODO --> change this to use strstr()?  & match with "->"

    char socket1[56];
    char socket2[56];
    char *end = strchr(socketTuple, '-');
    if (end) {
        size_t length = end - socketTuple;
            strncpy(socket1, socketTuple, length);
            socket1[length] = '\0'; // Null-terminate the extracted socket
    }
    //Skip over ->
    char *start = end + 2;
    end = strchr(socketTuple, '\0');
    if (end) {
        size_t length = end - start;
            strncpy(socket2, start, length);
            socket2[length] = '\0'; // Null-terminate the extracted socket
    }

    // Initialize subgraph
    Subgraph* subgraph = (Subgraph*)malloc(sizeof(Subgraph));
    subgraph->graphNum = graphNum;
    subgraph->currentfd = fd;
    subgraph->node_count = 0;
    subgraph->edge_count = 0;

    // Make remote node pointer
    Node* remote = (Node*)malloc(sizeof(Node));
    strncpy(remote->name, socket1, sizeof(socket1)-1);
    remote->fd = fd;
    remote->graphNum = graphNum;
    strncpy(remote->shape, "diamond", 7);
    subgraph->nodes[subgraph->node_count] = remote;
    subgraph->node_count++;

    // Make local node
    Node* local = (Node*)malloc(sizeof(Node));
    strncpy(local->name, socket2, sizeof(socket2)-1);
    local->fd = fd;
    local->graphNum = graphNum;
    strncpy(local->shape, "diamond", 7);
    subgraph->nodes[subgraph->node_count] = local;
    subgraph->node_count++;
    
    //  Connect two
    Edge* edge = (Edge*)malloc(sizeof(Edge));
    strncpy(edge->from, socket1, sizeof(socket1));
    strncpy(edge->to, socket2, sizeof(socket2));
    edge->graphNum = graphNum;
    strncpy(edge->syscall, "accept4", 7);
    subgraph->edges[subgraph->edge_count] = edge;
    subgraph->edge_count++;

    // Add to global list
    graphs[graphNum] = subgraph;
}


// Helper method to parse file argument with "<f>"
void parseFileName(const char *args, char *outputArgs) {
    char *start = strstr(args, "<f>");
    if (start) {
        start += 3; // Skip past "<f>"
        char *end = strchr(start, ')');
        if (end) {
            size_t length = end - start;
            strncpy(outputArgs, start, length);
            outputArgs[length] = '\0'; // Null-terminate the extracted path
        } else {
            strncpy(outputArgs, "Unknown File", 255);
        }
    } else {
        strncpy(outputArgs, args, 255); // If no "<f>", use the entire fd string
    }
}


// Helper method to parse socket tuple
void parseNetworkTuple(const char *args, char *outputArgs) {
    char *start = strstr(args, "tuple=");
    if (start) {
        start += 6; //Skip past tuple=
        char *end = strchr(start, ' ');
        if (end) {
            size_t length = end - start;
            strncpy(outputArgs, start, length);
            outputArgs[length] = '\0'; // Null-terminates extracted tuple
            } else {
                strncpy(outputArgs, "Unknown tuple", 255);
            }
    } else {
        strncpy(outputArgs, args, 255); //If no tuple use entire fd string? -> may want to remove
    }
}

// returns the FD as a int (aka fd=13<...>)
int formatFD(char *fdString) {
    if(strcmp(fdString, "<NA>") == 0){
        return -1;
    }
    long int output;
    output = strtol(fdString, NULL, 10);
    return output;
}

void parseLine(char line[], char *FD, char *syscall, char *args, char *ret, char *PID)
{
    // ignore timestamp, Information and process name (%*s)
    // store the file desc. (the number), syscall name, arugment string, all return values, and the PID string!!
    sscanf(line, "%*s %*s %*s FD:%[^,]  Syscall:%s Args:%[^,] Return:%[^,] PID:%[^\n]", FD, syscall, args, ret, PID);
    return;
}

bool parseSyscall(char syscall[], char returnValues[], char arguments[], char FD[]){
	//does NOT interact with a file...
	// || strcmp(syscall, "access") == 0
	if(strcmp(syscall, "rt_sigaction") == 0 || strcmp(syscall, "rt_sigprocmask") == 0 || strcmp(syscall, "brk") == 0 || strcmp(syscall, "munmap") == 0)
	{
		return false;
	}
	// chdir with <NA> does not touch a file path...
	else if(strcmp(syscall, "chdir") == 0 && strcmp(arguments, "") == 0)
	{
		return false;
	}
	// open with <NA> does not touch a file file path...
	//&& strcmp(returnValues, "<NA>") == 0
	// TODO --> this is resulting in the removal of the other PIDS BUT not fixing tracking the syscalls???
	else if (strcmp(syscall, "open") == 0 )
	{
		return false; 
	}
	// mmap with <NA> does not touch a file file path...
	else if(strcmp(syscall, "mmap") == 0 && strcmp(FD, "<NA>") == 0)
	{
		return false;
	}
	// close with no arguments is a duplicate call...
	else if(strcmp(syscall, "close") == 0 && (strcmp(returnValues, "0 ") == 0 || strcmp(arguments, "") == 0))
	{
		return false;
	}
    else if(strcmp(syscall, "access") == 0 && strcmp(arguments, "mode=0") == 0)
    {
		return false;
	}
    // Syscall:accept4 Args:flags=0 Return:<NA> PID:1239
    else if(strcmp(syscall, "accept4") == 0 && strcmp(returnValues, "<NA>") == 0 )
    {
        return false;    
    }
	// This is a system call that HAS information...
	else
	{
		return true;
	}
}


main(){
    FILE *file = fopen("events.txt", "r");
    if (!file) {
        perror("Failed to open events file");
        return 1;
    }

    char line[1024];
    //GET THE FULL LINE OF information...
    while (fgets(line, sizeof(line), file)) {
        
        char fdString[4], syscall[64], args[512], ret[5], PID[5];
        parseLine(line, fdString, syscall, args, ret, PID);

        int FD = formatFD(fdString);

        // valid accept4 call
        if(FD != -1 && strcmp(syscall, "accept4") == 0) {
            parseNetworkTuple(args, args);
            makeSubgraph(FD, args);
            graphNum +=1;
            continue;
        } 
    }

}