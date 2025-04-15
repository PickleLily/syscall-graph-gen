#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "graph_Generator.h"
// --------------------TODO Area---------------------------------------------------------
/*
    TODO --> the master PID is being set as the PID passed in... SHOULD we create one for each network tuple?
        Should we use the incoming IP addr. as our MASTER PID


    Potentially refine parseLine further
    Make sure memory management is efficient
        Try to dynamically start freeing up space so things can be more efficient
            1. We need to have a better way of storing the valid/nonvalid graphs
            2. We need to try to cut back on data being stored to limit size
            3. We should be dumping (not maintaining/printing) graphs that are not valid
                These are graphs that never got a solid connector between the network tuple and the PID

    Change update-edge to dynamically remove and add back the edge and make sure its the correct title?
        It seems we're generating some graphs with more than 2 edges (one solid one not) that actually are valid graphs


*/
// --------------------SET UP---------------------------------------------------------
#define MAX_SUBNODES 100
#define MAX_SUBEDGES 1000
#define MAX_SUBGRAPHS 100

// Global graph Reference
Subgraph* graphs[MAX_SUBGRAPHS];
int totalGraphs = -1;
int currentGraph = -1;


// ---------------------Functions --------------------------------------------------------

// Used to streamline the number of times malloc must be called for structs
void* createStruct(size_t structSize) {
    void* ptr = malloc(structSize);
    if(ptr == NULL) {
        fprintf(stderr, "Failed to allocate struct space");
        exit(EXIT_FAILURE);
    } else {
        return ptr;
    }
}

Node* createNode(char* args, int fd, char* shape, int nodeID, Subgraph* subgraph) {
    Node* newNode = (Node*)createStruct(sizeof(Node));
    strncpy(newNode->args, args, strnlen(args, 256)+1);
    newNode->fd = fd;
    strncpy(newNode->shape, shape, strnlen(shape, 128)+1);
    newNode->nodeID = nodeID;
    printf("Made node %s\n", newNode->args);
    subgraph->nodes[subgraph->node_count] = newNode;
    subgraph->node_count++;
    return newNode;
}

Edge* createEdge(int to, int from, int graphNum, char* syscall, char* edgeType, Subgraph* subgraph) {
    Edge *newEdge = (Edge*)createStruct(sizeof(Edge));
    newEdge->from = from;
    newEdge->to = to;
    newEdge->graphNum = currentGraph;
    strncpy(newEdge->syscall, syscall, strnlen(syscall, 64)+1);
    strncpy(newEdge->edgeType, edgeType, strnlen(edgeType, 128)+1); // Do we want to make this more variable IDK
    subgraph->edges[subgraph->edge_count] = newEdge;
    subgraph->edge_count++;
    return newEdge;
}

// Default create Node, Edge, & Subgraph
void add_edge(int from, int to, char *syscall) {
    // Get current subgraph as temp val
    Subgraph* graph = graphs[currentGraph];

    // Check max edges -> Default termination end case
    if (graph->edge_count >= MAX_SUBEDGES) {
        fprintf(stderr, "Error: Maximum edges exceeded.\n");
        exit(1);
    }

    // Check if edge already exists
    // TODO -> Try to make this more cost efficient but unsure if can be done
    for (int i = 0; i < graph->edge_count; i++) {
        // Duplicate edge found, do not add
        if (graph->edges[i]->from == from && graph->edges[i]->to == to && strcmp(graph->edges[i]->syscall, syscall) == 0) {
            return;
        }
    }	

    // TODO Else create new edge
    Edge* newEdge = createEdge(to, from, currentGraph, syscall, "solid", graph);

    // If we just added close, return fd to original PID FD, PID will always be node 3 (2)
    if(strcmp(syscall, "close") == 0) {
        if (graph->currentfd == graph->nodes[0]->fd) {
            graph->currentfd = -1; //TODO We cannot do this, we need to have ability to make subgraphs with multiple parts!
            graph->isValid = -1;
        } else {
            graph->currentfd = graph->nodes[0]->fd;
        }
    }
}

// Explicit function for reassigning temp connection between network socket and PID to full connection
// TODO -> May want to actually hard code this below and just do this within Main?
void update_edge(int edge, char *newcall, char *edge_type) {
    int subgraphID = currentGraph;
    Subgraph* graph = graphs[currentGraph];

    //Update edge (denoted by edgenum for now) with new syscall
    Edge* temp = graph->edges[edge];
    strncpy(temp->syscall, newcall, strlen(newcall)+1);
    strncpy(temp->edgeType, edge_type, strlen(edge_type)+1);
}

// TODO --> this is not secure... why?
int find_or_add_node(int fileDescriptor, char *args, char PID[], char shape[]) {  
    //get current subgraph
    int subgraphID = currentGraph;
    Subgraph* graph = graphs[currentGraph];
    
    int nodeCount = graph->node_count;

    // Check if network tuple
    char tuple[256];
    strncpy(tuple, graph->nodes[0]->args, strlen(graph->nodes[0]->args)+1);
    strncat(tuple, "->", 3);
    strncat(tuple, graph->nodes[1]->args, strlen(graph->nodes[1]->args)+1);
    if (strcmp(tuple, args) == 0) {
        return 1;
    }

    for (int i = 2; i < nodeCount; i++) {  
        //if we see a matching argument break out of the loop...
        // Need to adapt to find the tuple instance
        if (strcmp(graph->nodes[i]->args, args) == 0) {
            return i;
        }
    }

    if (nodeCount >= MAX_SUBNODES) {
        fprintf(stderr, "Error: Maximum nodes exceeded.\n");
        exit(1);
    }

    Node* newNode = createNode(args, fileDescriptor, "ellipse", graphs[currentGraph]->node_count, graph);
    // Update the currentFD we'll be looking for
    graph->currentfd = fileDescriptor;

    return newNode->nodeID;
}

int getSubgraphFD(int currentFD) {
    //go through the list of graphs globally
    for(int i = 0; i <= totalGraphs; i ++){
        //check the currentfd of each subgraph and return that graphs graphNUM
        if(currentFD == graphs[i]->currentfd && graphs[i]->isValid == 0){
            return graphs[i]->graphNum;
        }
    }
    // If we do not encounter this fd, assume we are at a point where the last call happened to come from this graph (MASSIVE ASSUMPTION YEAH)
    // Else do nothing
    return -1;
}

int getNodeFD(int currentFD) {
    for(int i = 1; i < graphs[currentGraph]->node_count; i++) {
        if (graphs[currentGraph]->nodes[i]->fd == currentFD) {
            return i;
        }
    }
}

Subgraph* initialize_subgraph(int fd, char *PID){
    Subgraph* subgraph = (Subgraph*)createStruct(sizeof(Subgraph));
    subgraph->graphNum = currentGraph;
    subgraph->isValid = 0;
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

    // Initialize subgraph
    Subgraph* subgraph = initialize_subgraph(fd, PID);
    graphs[currentGraph] = subgraph;

    // Make remote node pointer
    Node* remote = createNode(socket1, fd, "diamond", subgraph->node_count, subgraph);

    // Make local node
    Node* local = createNode(socket2, fd, "diamond", subgraph->node_count, subgraph);
    
    //  Connect two
    Edge* networkedge = createEdge(local->nodeID, remote->nodeID, currentGraph, "accept4", "solid", subgraph);

    // Make PID node
    Node* pid = createNode(PID, fd, "rectangle", subgraph->node_count, subgraph);

    // Connect PID node
    Edge* pidedge = createEdge(pid->nodeID, local->nodeID, currentGraph, "", "dashed", subgraph);
}

// Method to parse arg information into the file descriptor itself
void parseArgs(const char *args, char *output) {
    char *res = strstr(args, "res"); // Means the argument is a return value system call
    if (res) {
        strncpy(output, "Unknown tuple\0", 255); // Delimit
    } else {
        // Skip the end of the fd delimiter
        char *start = strstr(args, ">");
        if (start) {
            start += 1; // Skip past <f>=
            // See if we terminate the external () with ')'
            char *end = strchr(start, ')'); //TODO Make sure is secure
            if (end) {
                size_t length = end - start;
                strncpy(output, start, length); // Copy what we currently have
                output[length] = '\0'; // Terminate the rest of this string pre-emptively
                char *innerParenth = strstr(output, "("); // We then have to catch any potential inner parenthesis
                if(innerParenth) {
                    strncat(output, ")\0", 2); // Close out the parenthesis and adds the null delimiter
                }
            } else {
                strncpy(output, "Unknown tuple", 255);
            }
        } else {
            strncpy(output, "Unknown tuple", 255); //If no tuple use entire fd string? -> may want to remove
        }
    }
}

bool parseLine(char line[], int FD, char *syscall, char *args, char *ret, char *PID)
{
    // To fix issue of variable lines in Falco output and to only continue on with valid syscall lines, reducing the number of operations necessary overall
        // We assign temporary values of the FD, Syscall, Args, Return Val, and PID
        // Upon a successful read we then update the foreign values'
        // Values that may instictively be assigned to ints are instead stored as char arrays to better standardize potential outliers
        // char fdString[4], syscallString[64], argsString[1024], retString[64], pidString[64], program[64];
    // Upon a failure: Not 5-valid inputs, invalid FD
        // We default the FD to -1 to fall through subsequent checks in the program
    
    // TODO -> Make this more neat/refined if possible? IDk
        // Do we possibly want to start grabbing NAME out of this?
        // Also potentially exclude other erroneous FDs here
    char fdString[4];
    if((sscanf(line, "%*[^F]FD:%4[^,], Syscall:%64[^,], Args:%1024[^,], Return:%64[^,], PID:%64[^\n]", fdString, syscall, args, ret, PID) != 5)
        || (strncmp(fdString, "-1", 4) == 0) || (strncmp(fdString, "<NA>", 4) == 0)) {
        FD = -1;
        return false;
    } else {
        long int output;
        output = strtol(fdString, NULL, 10);
        FD = output;
        // printf("%d, %s, %s, %s, %s\n", FD, syscall, args, ret, PID);
        return true;
    }
}

// Method for filtering out additional lines that, while valid, do not contain data we can work with
bool parseSyscall(char syscall[], char returnValues[], char arguments[], char FD[]){
	if(strcmp(syscall, "rt_sigaction") == 0 || strcmp(syscall, "rt_sigprocmask") ==  0 || strcmp(syscall, "brk") == 0 || strcmp(syscall, "munmap") == 0)
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
    else if(strcmp(syscall, "write") == 0 )
    {
        return false;    
    }
	// This is a system call that HAS information...
	else
	{
		return true;
	}
}

void printSubgraphMetadata(){
    printf("%d subgraphs created\n", totalGraphs+1);
    for(int i = 0; i <= totalGraphs; i++){
        printf("graph %d Master PID: %d\n", i , graphs[i]->masterPID_ID);
        printf("nodes: %d    edges: %d\n\n", graphs[i]->node_count, graphs[i]->edge_count);
    }
}

// Helper method to parse socket tuple
// TODO Have this make all graphs as subgraphs within same larger file
void createDOT(char* setting){
    // Grab time to serve as naming convention
    time_t instance;
    instance = time(NULL);

    //Delimited by setting
    if(strcmp("individual", setting) == 0){
        //Make a subdirectory for all these graphs
        char makeCommand[256];

        sprintf(makeCommand, "mkdir \".\\Dot Files\\Timestamp_%d\"", &instance);
        if (system(makeCommand) == -1){ // Try the command
            perror("Could not make subdirectory for graphs");
        }
        for(int i = 0; i <= totalGraphs; i++){ //for every subgraph
        
            // open new dot file with unique name
            char path[1024];
            sprintf(path, ".\\Dot Files\\Timestamp_%d\\graph%d.dot", &instance, i);
            FILE *dot_file = fopen(path, "w");
    
            if (!dot_file) {
                perror("Failed to open DOT file\n");
                printf("%s\n", path);
                return;
            }
    
            //print the setup info:
            fprintf(dot_file, "digraph nginx_syscalls {\n");
    
            // add all of the nodes
            
            for(int j = 0; j < graphs[i]->node_count; j++){
                Node* n = graphs[i]->nodes[j];
                fprintf(dot_file, "  %d [label=\"%s\" shape=%s];\n", j, graphs[i]->nodes[j]->args, graphs[i]->nodes[j]->shape);
                // printf("  %d [label=\"%s\" shape=%s];\n", j, graphs[i]->nodes[j]->args, graphs[i]->nodes[j]->shape);
            }
    
            Edge** e = graphs[i]->edges;
            // add all of the edges
            for(int j = 0; j < graphs[i]->edge_count; j++){
                fprintf(dot_file, "  %d -> %d [style=\"%s\", label=\"%s\"];\n", graphs[i]->edges[j]->from, graphs[i]->edges[j]->to, graphs[i]->edges[j]->edgeType, graphs[i]->edges[j]->syscall);
                // printf("  %d -> %d [style=\"%s\", label=\"%s\"];\n", graphs[i]->edges[j]->from, graphs[i]->edges[j]->to, graphs[i]->edges[j]->edgeType, graphs[i]->edges[j]->syscall);
            }
    
            //print the shutdown info:
            fprintf(dot_file, "}\n");
            fclose(dot_file);
            // printf("Graph exported to %s\n", path);
    
        }

    } else if(strcmp("together", setting) == 0) {
        // Open dot file
        // open new dot file with unique name - generated randomly
        char path[1024];
        sprintf(path, "./Dot Files/Timestamp_%d.dot", &instance);
        printf("Created graph %s", path);
        FILE *dot_file = fopen(path, "w");

        if (!dot_file) {
            perror("Failed to open DOT file\n");
            return;
        }

        // After new dotfile has been successfully made:
        //print the setup info:
        fprintf(dot_file, "digraph nginx_syscalls {\n");

        for(int i = 0; i <= totalGraphs; i++){ //for every subgraph

            // Init subgraph
            fprintf(dot_file, "subgraph cluster_%d {\n", i);
        
            // add all of the nodes
            for(int j = 0; j < graphs[i]->node_count; j++){
                Node* n = graphs[i]->nodes[j];
                fprintf(dot_file, "  %d%d [label=\"%s\" shape=%s];\n", j, i, graphs[i]->nodes[j]->args, graphs[i]->nodes[j]->shape);
            }
    
            Edge** e = graphs[i]->edges;
            // add all of the edges
            for(int j = 0; j < graphs[i]->edge_count; j++){
                fprintf(dot_file, "  %d%d -> %d%d [label=\"%s\" style=%s];\n", graphs[i]->edges[j]->from, i, graphs[i]->edges[j]->to, i, graphs[i]->edges[j]->syscall, graphs[i]->edges[j]->edgeType);
            }

            // Close subgraph
            fprintf(dot_file, "}\n");
            // Insert another enter for readability
            fprintf(dot_file, "\n");
        }

        fprintf(dot_file, "}\n");
    } else if(strcmp("overlaid", setting) == 0) {
        // Open dot file
        // open new dot file with unique name - generated randomly
        char path[1024];
        sprintf(path, "./Dot Files/Timestamp_%d.dot", &instance);
        printf("Created graph %s", path);
        FILE *dot_file = fopen(path, "w");

        if (!dot_file) {
            perror("Failed to open DOT file\n");
            return;
        }

        // After new dotfile has been successfully made:
        //print the setup info:
        fprintf(dot_file, "digraph nginx_syscalls {\n");

        for(int i = 0; i <= totalGraphs; i++){ //for every subgraph

            // Init subgraph
            fprintf(dot_file, "subgraph cluster_%d {\n", i);
        
            // add all of the nodes
            for(int j = 0; j < graphs[i]->node_count; j++){
                Node* n = graphs[i]->nodes[j];
                fprintf(dot_file, "  %d [label=\"%s\" shape=%s];\n", j, graphs[i]->nodes[j]->args, graphs[i]->nodes[j]->shape);
            }
    
            Edge** e = graphs[i]->edges;
            // add all of the edges
            for(int j = 0; j < graphs[i]->edge_count; j++){
                fprintf(dot_file, "  %d -> %d [label=\"%s\" style=%s];\n", graphs[i]->edges[j]->from, graphs[i]->edges[j]->to, graphs[i]->edges[j]->syscall, graphs[i]->edges[j]->edgeType);
            }

            // Close subgraph
            fprintf(dot_file, "}\n");
            // Insert another enter for readability
            fprintf(dot_file, "\n");
        }

        fprintf(dot_file, "}\n");
    } else {
        createDOT("individual");
    }
    printf("Printed graph(s): %d", &instance);
}

int main(){

    FILE *file = fopen("./Falco Trace Files/TestEvents.txt", "r");
    if (!file) {
        perror("Failed to open events file");
        return 1;
    }

    char line[1024];
    int number_of_subgraph_nodes = 0;

    //GET THE FULL LINE OF information...
    while (fgets(line, sizeof(line), file)) {
        
        // Parse the syscalls from the line to our mapped values if the input is valid
        // Then check to ensure the valid values are what we are interested in
        // If both conditions are met, proceed
        char syscall[64], args[1024], ret[64], PID[64];
        int FD;
        if(parseLine(line, FD, syscall, args, ret, PID) && parseSyscall(syscall, ret, args, PID)) {

            // At each accept4 we want to start new subgraph
            if(strcmp(syscall, "accept4") == 0) {
                totalGraphs = totalGraphs + 1;
                currentGraph = totalGraphs;   
                // printf("There are now: %d graphs\n", totalGraphs);  // Used for debugging           
                parseArgs(args, args);
                makeSubgraph(FD, args, PID);
                number_of_subgraph_nodes = 0;
            } 

            // At any other system call we want to modify the graph we are currently working on
            else
            {
                if(totalGraphs >= 0){
                    // Increment nodes
                    number_of_subgraph_nodes+=1;

                    // printf("I am syscall:%s with args:%s and i have fd:%d, and the current graphs fd is:%d\n", syscall, args, FD, graphs[currentGraph]->currentfd);
                    
                    // Parse args
                    parseArgs(args, args);
                    if(strcmp("Unknown tuple", args) != 0) {
                        // See if we need to modify current graph
                        int tempCurrentGraph = getSubgraphFD(FD);

                        // If the file descriptor is brand new (its either -1)
                        // Do not change current graph, add node and edge
                        if(tempCurrentGraph == -1) { //TODO

                            int newNode = find_or_add_node(FD, args, PID, "ellipse");
                            add_edge(2, newNode, syscall);
                        // If the file descriptor has been run into before, we update the current graph and add an edge
                        } else if (tempCurrentGraph != -1) {
                                currentGraph = tempCurrentGraph; // Should put us on the correct subgraph
                                
                                // See if this is the first connection to this graph (Making it a Full Graph)
                                if(strcmp("dashed", graphs[currentGraph]->edges[1]->edgeType) == 0) { //TODO Connect()
                                    update_edge(1, syscall, "solid");

                                } else {
                                    // Get current fd node 
                                    int node = getNodeFD(FD);
                                    add_edge(2, node, syscall);

                                }
                        }
                    }
                }
            }
        }
    }
    // printSubgraphMetadata();
    createDOT("individual");
}







/*  Retired Code

Print the output of each graph --> Depricated
void printOutput() {
    int i, j = 0;
    for(int i = 0 ; i < totalGraphs; i ++){

        for(int j = 0 ; j < graphs[i]->node_count; j ++){
            printf("node:%s\n", graphs[i]->nodes[j]->args);
        }
        for(int j = 0 ; j < graphs[i]->edge_count; j ++){
            printf("edge: %s from:%d to:%d\n",graphs[i]->edges[j]->syscall, graphs[i]->edges[j]->from, graphs[i]->edges[j]->to);
        }
    }
}

Dedicated network tuple parsing function
void parseNetworkTuple(const char *arguments, char *from, char *to){
    //look for the end of the arrow signifying a connection between two IP's
    char *start = strstr(arguments, ">");
    if(start){
        int socket2start = start + 1;
        int socket2end = length(&arguments);

        int socket1start = 0;
        int socket1end = start - 1;
        strncpy(from, arguments[socket1start]);
        strncpy(to);
    }
    return;
}

Method for returning the FD as an int (aka fd=13<...>)
Was retired due to inclusion within original input checking step
int formatFD(char *fdString) {
    if(strcmp(fdString, "<NA>") == 0){
        return -1;
    }
    long int output;
    output = strtol(fdString, NULL, 10);
    return output;
}


*/
