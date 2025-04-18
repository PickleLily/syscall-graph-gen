# import ... as ...

# Global variables
MAX_SUBNODES = 100
MAX_SUBEDGES = 1000
MAX_SUBGRAPHS = 100
DOT_TYPE =  "individual"
DEBUG_LEVEL = 0

# Global graph Reference
graphs = [MAX_SUBGRAPHS];
totalGraphs = -1;
currentGraph = -1;

# // Default create Node, Edge, & Subgraph
# Node* createNode(char *args, int fd, char *shape, int nodeID, int nodePID, char *isProcess, Subgraph* subgraph) {
#     Node* newNode = (Node*)createStruct(sizeof(Node));
#     strncpy(newNode->args, args, strnlen(args, 255)+1);
#     newNode->fd = fd;
#     strncpy(newNode->shape, shape, strnlen(shape, 127)+1);
#     newNode->nodeID = nodeID;
#     newNode->nodePID = nodePID;
#     strncpy(newNode->process, isProcess, 2); // Default is NOT a process 'n\0'
#     // printf("Made node %s\n", newNode->args);
#     subgraph->nodes[subgraph->node_count] = newNode;
#     subgraph->node_count++;
#     printf("There are now %d nodes in graph %d\n", subgraph->node_count, currentGraph);
#     return newNode;
# }

class Node:
    def __init__(self, fd: int, shape, nodeID: int, nodePID: int, isProcess, subgraph):



# void* createStruct(size_t structSize) {
#     void* ptr = malloc(structSize);
#     if (ptr == NULL) {
#         fprintf(stderr, "Failed to allocate struct space");
#         exit(EXIT_FAILURE);
#     } else {
#         return ptr;
#     }
# }


# Helper method to open a file and ensure validity

# FILE* openFile(char *fileName, char *mode) {
#     FILE *file = fopen(fileName, mode);
#     if (!file) {
#         perror("Failed to open DOT file\n");
#         printf("%s\n", fileName);
#         exit(EXIT_FAILURE);
#     } else {
#         return file;
#     }
# }
