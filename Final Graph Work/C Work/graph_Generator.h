// graphGen.h
#ifndef GRAPHGEN_H
#define GRAPHGEN_H

// Structs 
typedef struct Node {
    char args[256];       // Name of the object (e.g., file path, socket info, subprocesses info?)
    int fd;               // The file descriptor
    char shape[128];
    int nodeID;
    int nodePID;
    char process[2];        // Is this node a process or just an FD
} Node;

typedef struct Edge {
    int from;       // name of the source node 
    int to;         // name of the destination node
    char bidirectional[2]; // Is the edge bidirectional (cuts down on space) n = no y = yes
    char syscall[64];     // The system call connecting the nodes
    char edgeType[8];   //"dashed", "dotted" ,"solid", "invis", "bold"
} Edge;

// Subgraph representation + functions
typedef struct Subgraph {
    int graphNum;           // Unique subgraph number
    int isValid;            // Marker to delimit if the graph is valid (editable, or has a complete connection)
    int currentfd[20];          // Currently active file descriptors -> We should not need more than 10
    int masterPID;          // The Node ID of the process that starts interactions?
    int masterRemote;       // Master Remote Connector
    Node* nodes[100];       // Hardcoded here
    Edge* edges[1000];      // Hardcoded here
    int node_count;
    int edge_count;
} Subgraph;

// Functions
void* createStruct(size_t structSize);
Node* createNode(char *args, int fd, char *shape, int nodeID, int nodePID, char *process, Subgraph* subgraph);
Edge* createEdge(int to, int from, char *syscall, char *edgeType, Subgraph* subgraph);
Subgraph* initializeSubgraph(int fd, char *PID);
void addEdge(int from, int to, char *syscall);
void updateEdge(Subgraph* subgraph, int edge, char *newcall, char *edge_type);
int findOrAddNode(int fileDescriptor, char *args, char PID[], char *process, char shape[]);
int getSubgraphFD(int currentFD, int currentPID);
int getProcessNode(int currentFD, int currentPID);
int getNodeFD(int currentFD, char *args, int currentPID);
void parseArgs(const char *args, char *output);
bool parseLine(char line[], int *FD, char *syscall, char *args, char *ret, char *PID);
bool parseSyscall(char syscall[], char returnValues[], char arguments[], char FD[]);
void makeSubgraph(int fd, char *remoteVal, char *localVal, char *PID);
void printSubgraphMetadata();
void createDOT(char *setting);

// Retired functions
// int formatFD(char *fdString);
// void parseNetworkTuple(char args[], char from[], char to[]);
// void printOutput();
// void createDOT();

#endif /* GRAPHGEN_H */
