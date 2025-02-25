#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
// --------------------TODO Area---------------------------------------------------------
/*
    TODO --> the master PID is being set as the PID passed in... SHOULD we create one for each network tuple?
        Should we use the incoming IP addr. as our MASTER PID




*/
// --------------------SET UP---------------------------------------------------------
#define MAX_SUBNODES 100
#define MAX_SUBEDGES 1000
#define MAX_SUBGRAPHS 100

// Structures for Nodes and Edges
typedef struct Node {
    char PID[64];         // PID
    char args[256];       // Name of the object (e.g., file path, socket info, subprocesses info?)
    int fd;               // The file descriptor
    int graphNum;         //Unique subgraph
    char shape[128];
    int nodeID;
} Node;

typedef struct Edge {
    int from;       // name of the source node 
    int to;         // name of the destination node
    int graphNum;         //Unique subgraph
    char syscall[64];     // The system call connecting the nodes
    char edgeType[128];   //"dashed", "dotted" ,"solid", "invis", "bold"
} Edge;

// Subgraph representation
typedef struct Subgraph {
    int graphNum;           //Unique subgraph number
    int currentfd;          //fd of the root accept4 node - not sure if we even need - shpuld this be current fd?
    int masterPID_ID;       //the Node ID of the process that starts interactions?
    Node* nodes[MAX_SUBNODES];
    Edge* edges[MAX_SUBEDGES];
    int node_count;
    int edge_count;
} Subgraph;

// Global graph ID
int graphNum = -1;

// Global graph Reference
Subgraph* graphs[MAX_SUBGRAPHS];

// ---------------------Functions --------------------------------------------------------

int getSubgraphFD(int currentFD){
    //go through the list of graphs globally
    for(int i = 0; i < graphNum; i ++){
        Subgraph *g = graphs[i];

        //check the currentfd of each subgraph and return that graphs graphNUM
        if(currentFD == g->currentfd){
            return g->graphNum;
        }
    }
    return -1;
}

Subgraph* initialize_subgraph(int fd, char *PID){
    Subgraph* subgraph = (Subgraph*)malloc(sizeof(Subgraph));
    subgraph->graphNum = graphNum;
    subgraph->currentfd = fd;
    subgraph->node_count = 0;
    subgraph->edge_count = 0;
    subgraph->masterPID_ID = atoi(PID);
    return subgraph;
}

// When supplied with fd of accept4 call, make new split graph
void makeSubgraph(int fd, char *socketTuple, char *PID) {
    // Take global graph num, current fd of accept4, and arg information to build the Subgraph
    // Predefine the two sockets
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
    printf("%s, %s\n", socket1, socket2);

    // Initialize subgraph
    Subgraph* subgraph = initialize_subgraph(fd, PID);
    int currentGraphNum = graphNum;
    graphs[currentGraphNum] = subgraph;

    // segmentation fault...
    int numNodes = graphs[currentGraphNum]->node_count;
    int numEdges = graphs[currentGraphNum]->edge_count;

    // Make remote node pointer
    Node* remote = (Node*)malloc(sizeof(Node));
    strncpy(remote->args, socket1, sizeof(socket1)-1);
    remote->fd = fd;
    remote->graphNum = graphNum;
    strncpy(remote->shape, "diamond", 7);
    subgraph->nodes[subgraph->node_count] = remote;
    remote->nodeID = graphs[graphNum]->node_count;
    subgraph->node_count++;
    // TODO find_or_add_node();

    // Make local node
    Node* local = (Node*)malloc(sizeof(Node));
    strncpy(local->args, socket2, sizeof(socket2)-1);
    local->fd = fd;
    local->graphNum = graphNum;
    strncpy(local->shape, "diamond", 7);
    subgraph->nodes[subgraph->node_count] = local;
    remote->nodeID = graphs[graphNum]->node_count;
    subgraph->node_count++;

    // find_or_add_node(socket2, PID, subgraph->node_count);

    
    //  Connect two
    Edge* networkedge = (Edge*)malloc(sizeof(Edge));
    // strncpy(networkedge->from, socket1, sizeof(socket1));
    // strncpy(networkedge->to, socket2, sizeof(socket2));
    networkedge->from = remote->nodeID;
    networkedge->to = local->nodeID;

    networkedge->graphNum = graphNum;
    strncpy(networkedge->syscall, "accept4", strlen("accept4"));
    subgraph->edges[subgraph->edge_count] = networkedge;
    strncpy(networkedge->edgeType, "solid", strlen("solid"));
    subgraph->edge_count++;

    // TODO add_edge();

    // Make PID node
    Node* pid = (Node*)malloc(sizeof(Node));
    strncpy(pid->args, PID, strlen(PID));
    pid->fd = fd;
    strncpy(pid->shape, "rectangle", 10);
    subgraph->nodes[subgraph->node_count] = pid;
    pid->nodeID = subgraph->node_count;
    subgraph->node_count++;

    // Connect PID node
    Edge* pidedge = (Edge*)malloc(sizeof(Edge));
    pidedge->from = local->nodeID;
    pidedge->to = pid->nodeID;
    // strncpy(pidedge->from, socket2, sizeof(socket2));
    // strncpy(pidedge->to, PID, sizeof(pid));

    pidedge->graphNum = graphNum;
    strncpy(pidedge->syscall, "", 1);
    subgraph->edges[subgraph->edge_count] = pidedge;
    strncpy(pidedge->edgeType, "dashed", strlen("dashed"));
    subgraph->edge_count++;

    // Add to global list
    graphs[graphNum] = subgraph;
}

// Method for printing the graph list
// TODO --> simple output... NOT sure if this is what we actually want to be printed...
void printOutput() {
    int i, j = 0;
    for(int i = 0 ; i < graphNum; i ++){

        for(int j = 0 ; j < graphs[i]->node_count; j ++){
            printf("node:%s\n", graphs[i]->nodes[j]->args);
        }
        for(int j = 0 ; j < graphs[i]->edge_count; j ++){
            printf("edge: %s from:%d to:%d\n",graphs[i]->edges[j]->syscall, graphs[i]->edges[j]->from, graphs[i]->edges[j]->to);
        }
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

// Helper method to parse arg information
void parseArgs(const char *args, char *format, char *output) {
    if(strcmp(format, "network socket") || strcmp(format, "file name")){
        char *start = strstr(args, ">");
        if (start) {
            start += 1; //Skip past <f>=
            char *end = strchr(start, ')');
            if (end) {
                size_t length = end - start;
                strncpy(output, start, length);
                output[length] = '\0'; // Null-terminates extracted tuple
                } else {
                    strncpy(output, "Unknown tuple", 255);
                }
        } else {
            strncpy(output, args, 255); //If no tuple use entire fd string? -> may want to remove
        }
    }
}

void parseLine(char line[], char *FD, char *syscall, char *args, char *ret, char *PID)
{
    // ignore timestamp, Information and process name (%*s)
    // store the file desc. (the number), syscall name, arugment string, all return values, and the PID string!!
    char time[64];
    char type[64];
    char program[64];
    sscanf(line, "%s %s Name:%s FD:%[^ ] Syscall:%[^ ] Args:%[^,], Return:%[^,], PID:%[^\n]", time, type, program, FD, syscall, args, ret, PID);
    return;
}

bool parseSyscall(char syscall[], char returnValues[], char arguments[], char FD[]){

	if(strcmp(syscall, "rt_sigaction") == 0 || strcmp(syscall, "rt_sigprocmask") == 0 || strcmp(syscall, "brk") == 0 || strcmp(syscall, "munmap") == 0)
	{
		return false;
	}
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
	else if(strcmp(syscall, "mmap") == 0 && strcmp(FD, "<NA>") == 0)
	{
		return false;
	}
	else if(strcmp(syscall, "close") == 0 && (strcmp(returnValues, "0 ") == 0 || strcmp(arguments, "") == 0))
	{
		return false;
	}
    else if(strcmp(syscall, "access") == 0 && strcmp(arguments, "mode=0") == 0)
    {
		return false;
	}
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

void printSubgraphs(){

    printf("%d subgraphs created\n", graphNum);
    for(int i = 0; i < graphNum; i++){
        printf("graph %d Master PID: %d\n",i,graphs[graphNum]->masterPID_ID);
        printf("nodes: %d    edges: %d\n\n",graphs[graphNum]->node_count, graphs[graphNum]->edge_count);
    }
}

// Helper method to parse socket tuple
void createDOT(){

    for(int i = 0; i < graphNum; i++){ //for every subgraph

        // open new dot file with unique name
        char path[1024];
        sprintf(path, "./graphs/graph%d.dot", i);
        printf("%s", path);
        FILE *dot_file = fopen(path, "w");

        if (!dot_file) {
            perror("Failed to open DOT file\n");
            return;
        }

        //print the setup info:
        fprintf(dot_file, "digraph nginx_syscalls {\n");


        // add all of the nodes
        for(int j = 0; j < graphs[graphNum]->node_count; j++){
            fprintf(dot_file, "  %d [label=\"%s\" shape=%s];\n", j, graphs[i]->nodes[j]->args, graphs[i]->nodes[j]->shape);
        }
        // add all of the edges
        for(int j = 0; j < graphs[graphNum]->edge_count; j++){
            fprintf(dot_file, "  %d -> %d [label=\"%s\"];\n", graphs[i]->edges[j]->from, graphs[i]->edges[j]->to, graphs[i]->edges[j]->syscall);
        }


        //print the shutdown info:
        fprintf(dot_file, "}\n");
        fclose(dot_file);
        printf("Graph exported to %s\n", path);

    }
}

int main(){

    FILE *file = fopen("events.txt", "r");
    if (!file) {
        perror("Failed to open events file");
        return 1;
    }

    char line[1024];
    bool logging = false;

    //GET THE FULL LINE OF information...
    while (fgets(line, sizeof(line), file)) {
        
        //store the arguments from the line
        char fdString[4], syscall[64], args[1024], ret[64], PID[64];
        parseLine(line, fdString, syscall, args, ret, PID);

        //make the FD an int
        int FD = formatFD(fdString);

        // if we have a file interacted with
        if(FD != -1){
            // if it is accept4
            if(strcmp(syscall, "accept4") == 0) 
            {
                graphNum +=1;                
                parseArgs(args, "network socket", args);
                makeSubgraph(FD, args, PID);
                continue;
            } 
            // if it is any other syscall
            else
            {
                if(graphNum >= 0){
                    // printf("graph %d: %s\n", graphNum, syscall);
                //     for(int i = 0; i < graphNum-1; i++) {
                //         if(graphs[i]->currentfd == FD){
                //             printf("should add edge\n");
                //             // add_edge(from, to, syscall);
                //         }
                //     }
                }
            }
        }
    }
    printSubgraphs();
    printOutput();
    createDOT();
}

// ------------------------ The pit (old functions not being used) ----------------------------------------------------------------

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


void add_edge(int from, int to, const char *syscall) {
    //get current subgraph
    int subgraphID = graphNum;
    Subgraph* graph = graphs[graphNum];
    Edge **edges = graph->edges; // A list of edge pointers

    int lengthEdges = graph->edge_count;
    
    //check if edge exists
    for (int i = 0; i < lengthEdges; i++) {
        // Ella's (AI GODS) tips: If ever pulling an exact value out of a pointer we need to derefernce
        // Now that these are also chars we dont need to worry about it
        if (edges[i]->from == from && edges[i]->to == to && strcmp(edges[i]->syscall, syscall) == 0) {
            return; // Duplicate edge found, do not add
        }
    }	

    //check max edges
    if (lengthEdges >= MAX_SUBEDGES) {
        fprintf(stderr, "Error: Maximum edges exceeded.\n");
        exit(1);
    }

    graph->edge_count++;
    edges[lengthEdges]->from = from;
    edges[lengthEdges]->to = to;
    strncpy(edges[lengthEdges]->syscall, syscall, strlen(syscall));
    strncpy(edges[lengthEdges]->edgeType, "solid", strlen("solid"));
}

int find_or_add_node(const char *args, char PID[], char shape[]) {  
    
    // //get current subgraph
    // int subgraphID = graphNum;
    // Subgraph* graph = graphs[graphNum];
    
    // //TODO --> this is segfaulting...
    // int nodeCount = graph->node_count;

    // for (int i = 0; i < nodeCount; i++) {  
    //     //if we see a matching argument break out of the loop...
    //     if (strcmp(graph->nodes[i]->args, args) == 0) {
    //         return graph->nodes[i]->fd;
    //     }
    //     printf("found node\n");
    // }

    // if (nodeCount >= MAX_SUBNODES) {
    //     fprintf(stderr, "Error: Maximum nodes exceeded.\n");
    //     exit(1);
    // }

    // graph->nodes[nodeCount]->fd = *node_count;
    // strncpy(nodes[nodeCount]->PID, PID, sizeof(nodes[nodeCount]->PID) - 1);
    // strncpy(nodes[nodeCount]->args, args, sizeof(nodes[nodeCount]->args) - 1);
    // graph->node_count++;

    // printf("found node\n");
    
    // return nodes[nodeCount]->fd;
}