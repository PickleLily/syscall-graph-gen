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
/* 
Debug level 0 is default: Prints no additional information
Debug level 1: Prints metadata for subgraphs
Debug level 2: Prints information about graphs as they are being built

Dot Type "individual" creates a dot file for each subgraph
Dot Type "together" creates a single dot file that contains every subgraph
*/
#define DOT_TYPE "individual"
#define DEBUG_LEVEL 0

// Global graph Reference
Subgraph* graphs[MAX_SUBGRAPHS];
int totalGraphs = -1;
int currentGraph = -1;


// ---------------------Functions --------------------------------------------------------

// Helper method to malloc structs and ensure validity
void* createStruct(size_t structSize) {
    void* ptr = malloc(structSize);
    if(ptr == NULL) {
        fprintf(stderr, "Failed to allocate struct space");
        exit(EXIT_FAILURE);
    } else {
        return ptr;
    }
}

// Helper method to open a file and ensure validity
FILE* openFile(char* fileName, char* mode) {
    FILE *file = fopen(fileName, mode);
    if (!file) {
        perror("Failed to open DOT file\n");
        printf("%s\n", fileName);
        exit(EXIT_FAILURE);
    } else {
        return file;
    }
}

// Default create Node, Edge, & Subgraph
Node* createNode(char* args, int fd, char* shape, int nodeID, Subgraph* subgraph) {
    Node* newNode = (Node*)createStruct(sizeof(Node));
    strncpy(newNode->args, args, strnlen(args, 255)+1);
    newNode->fd = fd;
    strncpy(newNode->shape, shape, strnlen(shape, 127)+1);
    newNode->nodeID = nodeID;
    printf("Made node %s\n", newNode->args);
    subgraph->nodes[subgraph->node_count] = newNode;
    subgraph->node_count++;
    return newNode;
}

Edge* createEdge(int to, int from, char* syscall, char* edgeType, Subgraph* subgraph) {
    Edge *newEdge = (Edge*)createStruct(sizeof(Edge));
    newEdge->from = from;
    newEdge->to = to;
    strncpy(newEdge->syscall, syscall, strnlen(syscall, 63)+1);
    strncpy(newEdge->edgeType, edgeType, strnlen(edgeType, 7)+1); // Do we want to make this more variable IDK
    subgraph->edges[subgraph->edge_count] = newEdge;
    subgraph->edge_count++;
    return newEdge;
}

Subgraph* initializeSubgraph(int fd, char *PID){
    Subgraph* subgraph = (Subgraph*)createStruct(sizeof(Subgraph));
    subgraph->graphNum = currentGraph;
    subgraph->isValid = 0;
    subgraph->currentfd = fd;
    subgraph->node_count = 0;
    subgraph->edge_count = 0;
    subgraph->masterPID = atoi(PID);
    subgraph->masterRemote = atoi(PID);
    return subgraph;
}

void addEdge(int from, int to, char *syscall) {
    // Get current subgraph as temp val
    Subgraph* graph = graphs[currentGraph];

    // Check max edges -> Default termination end case
    if (graph->edge_count >= MAX_SUBEDGES) {
        fprintf(stderr, "Error: Maximum edges exceeded.\n");
        exit(1);
    }

    // Check if edge already exists
    // TODO -> Try to make this more cost efficient but unsure if can be done
    // Working concepts:
        /*
        We skip 0,1 because those are handled separately
        */
    for (int i = 2; i < graph->edge_count; i++) {
        // Duplicate edge found, do not add
        if (graph->edges[i]->from == from && graph->edges[i]->to == to && strcmp(graph->edges[i]->syscall, syscall) == 0) {
            return;
        }
    }	

    // Else create new edge
    Edge* newEdge = createEdge(to, from, syscall, "solid", graph);

    // TODO
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
void updateEdge(int edge, char *newcall, char *edge_type) {
    // Get current graph
    Subgraph* graph = graphs[currentGraph];

    //Update edge (denoted by edgenum for now) with new syscall
    Edge* temp = graph->edges[edge];
    strncpy(temp->syscall, newcall, strlen(newcall)+1);
    strncpy(temp->edgeType, edge_type, strlen(edge_type)+1);
}

// TODO --> this is not secure... why?
int findOrAddNode(int fileDescriptor, char *args, char PID[], char shape[]) {  
    // Get current subgraph
    Subgraph* graph = graphs[currentGraph];

    // Default end case
    if (graph->node_count >= MAX_SUBNODES) {
        fprintf(stderr, "Error: Maximum nodes exceeded.\n");
        exit(1);
    }

    // Check if network tuple
    char tuple[256];
    strncpy(tuple, graph->nodes[0]->args, strlen(graph->nodes[0]->args)+1);
    strncat(tuple, "->", 3);
    strncat(tuple, graph->nodes[1]->args, strlen(graph->nodes[1]->args)+1);
    if (strcmp(tuple, args) == 0) {
        return 1;
    }

    // Find Node
    for (int i = 2; i < graph->node_count; i++) {  
        // If we see a matching argument break out of the loop...
        // Need to adapt to find the tuple instance
        if (strcmp(graph->nodes[i]->args, args) == 0) {
            return i;
        }
    }

    // Else Add Node
    Node* newNode = createNode(args, fileDescriptor, "ellipse", graphs[currentGraph]->node_count, graph);
    
    // Update the currentFD we'll be referencing
    graph->currentfd = fileDescriptor;
    return newNode->nodeID;
}

int getSubgraphFD(int currentFD) {
    // Go through the list of graphs globally
    for(int i = 0; i <= totalGraphs; i ++){
        // Check the currentfd of each subgraph and return that graphs graphNum
        if(currentFD == graphs[i]->currentfd && graphs[i]->isValid == 0){
            return graphs[i]->graphNum;
        }
    }
    // If we do not encounter this fd, assume we are at a point where the last call happened to come from this graph
    return -1;
}

// Helper function for finding the FD we need to be interacting with
int getNodeFD(int currentFD) {
    for(int i = 1; i < graphs[currentGraph]->node_count; i++) {
        if (graphs[currentGraph]->nodes[i]->fd == currentFD) {
            return i;
        }
    }
}

// When supplied with fd of accept4 call, make new split graph
void makeSubgraph(int fd, char *remoteVal, char *localVal, char *PID) {
    // Take global graph num, current fd of accept4, and arg information to build the Subgraph

    // Initialize subgraph
    totalGraphs = totalGraphs + 1;
    currentGraph = totalGraphs;
    printf("New total graphs: %d\n", totalGraphs);
    Subgraph* subgraph = initializeSubgraph(fd, PID);
    graphs[currentGraph] = subgraph;

    // Make remote node pointer
    Node* remote = createNode(remoteVal, fd, "diamond", subgraph->node_count, subgraph);

    // Make local node
    Node* local = createNode(localVal, fd, "diamond", subgraph->node_count, subgraph);
    
    //  Connect two
    Edge* networkedge = createEdge(local->nodeID, remote->nodeID, "accept4", "solid", subgraph);

    // Make PID node
    Node* pid = createNode(PID, fd, "rectangle", subgraph->node_count, subgraph);

    // Connect PID node
    Edge* pidedge = createEdge(pid->nodeID, local->nodeID, "", "dashed", subgraph);
}

// Method to parse arg information into the file descriptor itself
void parseArgs(const char *args, char *output) {
    char *res = strstr(args, "res"); // Means the argument is a return value system call
    size_t totalLength;
    if (res) {
        strncpy(output, "Unknown tuple\0", 255); // Delimit
    } else {
        // Skip the end of the fd delimiter
        char *start = strstr(args, ">");
        if (start) {
            start += 1; // Skip past <f>=
            // See if we terminate the external () with ')'
            char *end = strchr(start, ')'); //This returns the first instance of )
            if (end) {
                totalLength = end - start;
                memmove(output, start, totalLength); // Copy what we currently have
                output[totalLength] = '\0'; // Terminate the rest of this string pre-emptively
                char *innerParenth = strstr(output, "("); // We then have to catch any potential inner parenthesis
                if(innerParenth) {
                    strncat(output, ")\0", 2); // Close out the parenthesis and adds the null delimiter
                }
            } else {
                strncpy(output, "Unknown tuple\0", 255);
            }
        } else {
            strncpy(output, "Unknown tuple\0", 255); //If no tuple use entire fd string? -> may want to remove
        }
    }
    // Parse any sockets (->) into a simple comma seperator
    char *separator = strstr(output, "->");
    if (separator) {
        *separator = ',';
        size_t sublength = separator - output; // How far is the start of separator from the start of output
        // printf("%d\n", sublength);
        memmove(separator+1, separator+2, sublength);
        output[totalLength-1] = '\0';
        // printf("%s", output);
    }
}

// Parse information about each line of the debug we receive from Falco
// We only want to process/proceed with data that fits the constraints of having adequetly sized FD, Syscal, Args, Return, & PID
// Additionally, we want to ignore any debug lines with an FD of -1 or <NA>
// Finally, we want to call parseSyscall to once again reduce the number of operations we perform on potentially invalid data
bool parseLine(char line[], int *FD, char *syscall, char *args, char *ret, char *PID) {    
    // TODO -> Make this more neat/refined if possible? IDk
    char fdString[4];
    if((sscanf(line, "%*[^F]FD:%4[^,], Syscall:%64[^,], Args:%1024[^,], Return:%64[^,], PID:%64[^\n]", fdString, syscall, args, ret, PID) != 5)
        || (strncmp(fdString, "-1", 4) == 0) || (strncmp(fdString, "<NA>", 4) == 0)
        || !(parseSyscall(syscall, ret, args, PID))) {
        *FD = -1;
        return false;
    } else {
        long int output;
        output = strtol(fdString, NULL, 10);
        *FD = output;
        // printf("%d, %s, %s, %s, %s\n", FD, syscall, args, ret, PID);
        return true;
    }
}

// Method for filtering out additional lines that, while valid, do not contain data we can work with
bool parseSyscall(char syscall[], char returnValues[], char arguments[], char FD[]){
	// System calls that additionally should be ignored
    // if(strcmp(syscall, "rt_sigaction") == 0 
    // || strcmp(syscall, "rt_sigprocmask") ==  0 
    if(strcmp(syscall, "brk") == 0 
    || strcmp(syscall, "munmap") == 0
    || strcmp(syscall, "open") == 0 
    || strcmp(syscall, "write") == 0 ){
        return false;
    }
    // System calls that when having a specific argument should be ignored
	else if(strcmp(syscall, "chdir") == 0 && strcmp(arguments, "") == 0
    || strcmp(syscall, "access") == 0 && strcmp(arguments, "mode=0") == 0){
        return false;
    }
    // System calls that when having a specific return value should be ignored
	else if(strcmp(syscall, "close") == 0 && (strcmp(returnValues, "0 ") == 0 || strcmp(arguments, "") == 0)
    || strcmp(syscall, "accept4") == 0 && strcmp(returnValues, "<NA>") == 0){
        return false;
    }
	// This is a system call that HAS information...
	else{return true;}
}

void handleConnectionSystemCall(char *syscall, int FD, char *args, char *PID) {
    // In the occassion of receiving an accept4, accept, pipe, and connect
    /*
    When receiving an accept4 -> we have to make sure the tuple doesn't already exist, if so we have to assume the first one has been terminated?
    We need to be able to keep track of essentially a subgraph in a subgraph->closing out something like an SQL

    */
   printf("Total graphs: %d\n", totalGraphs);
    if(strcmp("accept4", syscall) == 0){
        char socket1[56];
        char socket2[56];
        char *end = strchr(args, ',');
        if (end) {
            size_t length = end - args;
            strncpy(socket1, args, length);
            socket1[length] = '\0'; // Null-terminate the extracted socket
        }
        // Skip over ->
        char *start = end + 1;
        end = strchr(args, '\0');
        if (end) {
            size_t length = end - start;
            strncpy(socket2, start, length);
            socket2[length] = '\0'; // Null-terminate the extracted socket
        }
        for(int i = 0; i <= totalGraphs; i++) {
            if(graphs[i]->isValid == 0) {
                if(strcmp(graphs[i]->nodes[0]->args, socket1) == 0){
                    graphs[i]->isValid = -1;
                    printf("Made graph: %d invalid\n", i);
                    break;
                }
            }
        }
        makeSubgraph(FD, socket1, socket2, PID);
    }
    else if(strcmp("accept", syscall) == 0){}
}


void printSubgraphMetadata(){
    printf("%d subgraphs created\n", totalGraphs+1); // Pad out graph 0 for human understanding
    for(int i = 0; i <= totalGraphs; i++){
        printf("graph %d Master PID: %d\n", i , graphs[i]->masterPID);
        printf("nodes: %d    edges: %d\n\n", graphs[i]->node_count, graphs[i]->edge_count);
    }
}

// Helper method to parse socket tuple
// TODO Have this make all graphs as subgraphs within same larger file
void createDOT(char* setting){
    // Grab time to serve as naming convention
    time_t current_time;
    struct tm *info;
    char timeBuffer[26];

    current_time = time(NULL);
    info = localtime(&current_time);

    strftime(timeBuffer, 26, "%Y_%m_%d__%H_%M_%S", info);


    //Determined by setting
    if(strcmp("individual", setting) == 0){
        
        //Make a subdirectory for all these graphs
        char makeCommand[256];
        sprintf(makeCommand, "mkdir \".\\Dot_Files\\Timestamp_%s\"", timeBuffer);
        if (system(makeCommand) == -1){ // Try the command
            perror("Could not make subdirectory for graphs");
        }
        for(int i = 0; i <= totalGraphs; i++){ // Do following for every subgraph
        
            // Open new dot file with unique name
            char path[1024];
            sprintf(path, ".\\Dot_Files\\Timestamp_%s\\graph%d.dot", timeBuffer, i);
            printf("%s\n", path);
            FILE *dot_file = openFile(path, "w+");
    
            // Print the setup info:
            fprintf(dot_file, "digraph nginx_syscalls {\n");
            fprintf(dot_file, "rankdir=LR;\n");
    
            // Print all nodes
            for(int j = 0; j < graphs[i]->node_count; j++){
                Node* n = graphs[i]->nodes[j];
                fprintf(dot_file, "  %d [label=\"%s\" shape=%s];\n", j, graphs[i]->nodes[j]->args, graphs[i]->nodes[j]->shape);
                // printf("  %d [label=\"%s\" shape=%s];\n", j, graphs[i]->nodes[j]->args, graphs[i]->nodes[j]->shape);
            }

            // Print all edges
            for(int j = 0; j < graphs[i]->edge_count; j++){
                fprintf(dot_file, "  %d -> %d [style=\"%s\", label=\"%s\", minlen=2, weight=2];\n", graphs[i]->edges[j]->from, graphs[i]->edges[j]->to, graphs[i]->edges[j]->edgeType, graphs[i]->edges[j]->syscall);
                // printf("  %d -> %d [style=\"%s\", label=\"%s\"];\n", graphs[i]->edges[j]->from, graphs[i]->edges[j]->to, graphs[i]->edges[j]->edgeType, graphs[i]->edges[j]->syscall);
            }
    
            //Print the EOF info & close file:
            fprintf(dot_file, "}\n");
            fclose(dot_file);
    
        }

    } else if(strcmp("together", setting) == 0) {
        // Open dot file
        // open new dot file with unique name - generated randomly
        char path[1024];
        sprintf(path, "./Dot_Files/Timestamp_%s.dot", timeBuffer);
        printf("Created graph %s", path);
        FILE *dot_file = openFile(path, "w");

        // After new dotfile has been successfully made:
        // Print the setup info:
        fprintf(dot_file, "digraph nginx_syscalls {\n");
        for(int i = 0; i <= totalGraphs; i++){ //for every subgraph

            // Initiate subgraph
            fprintf(dot_file, "subgraph cluster_%d {\n", i);
        
            // Print all nodes
            for(int j = 0; j < graphs[i]->node_count; j++){
                Node* n = graphs[i]->nodes[j];
                fprintf(dot_file, "  %d%d [label=\"%s\" shape=%s];\n", j, i, graphs[i]->nodes[j]->args, graphs[i]->nodes[j]->shape);
            }
   
            // Print all edges
            for(int j = 0; j < graphs[i]->edge_count; j++){
                fprintf(dot_file, "  %d%d -> %d%d [label=\"%s\" style=%s];\n", graphs[i]->edges[j]->from, i, graphs[i]->edges[j]->to, i, graphs[i]->edges[j]->syscall, graphs[i]->edges[j]->edgeType);
            }

            // Close subgraph
            fprintf(dot_file, "}\n");
            // Insert another enter for readability
            fprintf(dot_file, "\n");
        }
        // Close DOT file
        fprintf(dot_file, "}\n");
    } else {
        // Catch all other subcases/incorrect inputs and simply map them to "individual"
        createDOT("individual");
    }
    // Print final output of what folder/graph is called
    printf("Printed graph(s): %s", &timeBuffer);
}

int main(){

    FILE *file = openFile("./Falco Trace Files/TestEvents.txt", "r");
    char line[1024];

    // Get FULL line of information...
    while (fgets(line, sizeof(line), file)) {
        
        // Parse the syscalls from the line to our mapped values if the input is valid
        // Then check to ensure the valid values are what we are interested in
        // If both conditions are met, proceed
        char syscall[64], args[1024], ret[64], PID[64];
        int FD;
        if(parseLine(line, &FD, syscall, args, ret, PID)) {
            // Begin by parsing arguments to better refine
            parseArgs(args, args);

            // Check to see if we want to start new subgraph/handle other speciality cases
            // TODO
            if(strcmp(syscall, "accept4") == 0) {  
                // printf("What: %d, %s, %s", FD, args, PID);
                handleConnectionSystemCall(syscall, FD, args, PID);
            } 

            // At any other system call we want to modify the graph we are currently working on
            else
            {
                if(totalGraphs >= 0){
                    
                    if(strcmp("Unknown tuple", args) != 0) {
                        // See if we need to modify current graph
                        int tempCurrentGraph = getSubgraphFD(FD);

                        // If the file descriptor is brand new (its either -1)
                        // Do not change current graph, add node and edge
                        if(tempCurrentGraph == -1) { //TODO
                            int newNode = findOrAddNode(FD, args, PID, "ellipse");
                            addEdge(2, newNode, syscall);

                        // If the file descriptor has been run into before, we update the current graph and add an edge
                        } else if (tempCurrentGraph != -1) {
                                currentGraph = tempCurrentGraph; // Should put us on the correct subgraph
                                
                                // See if this is the first connection to this graph (Making it a Full Graph)
                                if(strcmp("dashed", graphs[currentGraph]->edges[1]->edgeType) == 0) { //TODO Connect()
                                    updateEdge(1, syscall, "solid");

                                } else {
                                    // Get current fd node 
                                    int node = getNodeFD(FD);
                                    addEdge(2, node, syscall);

                                }
                        }
                    }
                }
            }
        }
    }
    // Include additional debugging information if desired
    if(DEBUG_LEVEL == 1) {
        printSubgraphMetadata();
    }

    // Create graphs
    createDOT(DOT_TYPE);
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
