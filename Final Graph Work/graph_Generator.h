// graphGen.h
#ifndef GRAPHGEN_H
#define GRAPHGEN_H

// Structs 
typedef struct Node {
    char args[256];       // Name of the object (e.g., file path, socket info, subprocesses info?)
    int fd;               // The file descriptor
    // int graphNum;         //Unique subgraph
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

// Subgraph representation + functions
typedef struct Subgraph {
    int graphNum;           //Unique subgraph number
    int isValid;            // Marker to delimit if the graph is valid (editable, or has a complete connection)
    int currentfd;          //fd of the root accept4 node - not sure if we even need - shpuld this be current fd?
    int masterPID_ID;       //the Node ID of the process that starts interactions?
    Node* nodes[100];       // Hardcoded here
    Edge* edges[1000];      // Hardcoded here
    int node_count;
    int edge_count;
} Subgraph;

// Functions
void* createStruct(size_t structSize);
Node* createNode(char* args, int fd, char* shape, int nodeID, Subgraph* subgraph);
Edge* createEdge(int to, int from, int graphNum, char* syscall, char* edgeType, Subgraph* subgraph);
void add_edge(int from, int to, char *syscall);
void update_edge(int edge, char *newcall, char *edge_type);
int find_or_add_node(int fileDescriptor, char *args, char PID[], char shape[]);
int getSubgraphFD(int currentFD);
int getNodeFD(int currentFD);
void parseArgs(const char *args, char *output);
bool parseLine(char line[], int FD, char *syscall, char *args, char *ret, char *PID);
bool parseSyscall(char syscall[], char returnValues[], char arguments[], char FD[]);

Subgraph* initialize_subgraph(int fd, char *PID);
void makeSubgraph(int fd, char *socketTuple, char *PID);
int getSubgraphFD(int currentFD);
void printSubgraphMetadata();
void createDOT(char* setting);

// Retired functions
// int formatFD(char *fdString);
// void parseNetworkTuple(char args[], char from[], char to[]);
// void printOutput();
// void createDOT();

#endif /* GRAPHGEN_H */
