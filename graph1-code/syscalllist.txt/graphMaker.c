#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "graphMaker.h"
#define MAX_CONTEXT 20
#define NUM_SYSCALLS 373
#define MAX_EDGES 2000

int findNum(char *x){
    int min = 0;
    int max = NUM_SYSCALLS-1;
    while(min <= max){
        int mid = min + (max - min) / 2;
        //Calculate if our syscall is smaller or greater than middle syscall
        int eval = strcmp(x, syscallNames[mid]);

        //See if current string is correct
        if(eval == 0){
            return syscallNumbers[mid];
        }
        //Binary search
        //If our syscall is smaller we want to look at left side
        if(eval < 0){
            max = mid - 1;
        }
        //If our syscall is larger we want to look at right side
        else {
            min = mid + 1;
        }
    }
    //On error  
    return -1; 
}

typedef struct Node {
    int sysCallNum; //The system call number for this specific call
    struct Node* parent; //Points backwards to last call
    struct Node** children; //Points to an array of all potential following syscalls
    size_t numChildren; //The number of following syscalls
    struct Edge** edges;
    size_t numEdges;
} Node;

typedef struct Edge {
    Node* parent;
    Node* child;
} Edge;

Edge* makeEdge(Node* parent, Node* child){
    Edge* newEdge = (Edge*)malloc(sizeof(Edge));
    newEdge->parent = parent;
    newEdge->child = child;
    return newEdge;
}

Node* makeNode(int syscall) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->sysCallNum = syscall;
    newNode->parent = NULL;
    newNode->children = NULL;
    newNode->numChildren = 0;
    newNode->edges = NULL;
    newNode->numEdges = 0;
    return newNode;
}

void addNode(Node* parent, Node* child){
    if(parent == NULL || child == NULL){return;}
    parent->numChildren++; //Increment number of following nodes for parent
    //Realloc (free and malloc) the old pointer to pointers array with updated size
    parent->children = (Node**)realloc(parent->children, parent->numChildren * sizeof(Node*));
    parent->children[parent->numChildren - 1] = child; //Adds the child node to the end of parent's child node list
    
    parent->numEdges++;
    // child->numEdges++;
    parent->edges = (Edge**)realloc(parent->edges, parent->numEdges * sizeof(Edge*));
    Edge* newEdge = makeEdge(parent, child);
    // child->edges = (Edge**)realloc(child->edges, child->numEdges * sizeof(Edge*));
    parent->edges[parent->numEdges-1] = newEdge;
    child->parent = parent;
}

Node* setCurrentNode(int syscall, Node* current){
    Node* temp = current;
    //Check if up
    while(temp != NULL){
        //In the occassion the node we are looking at is the syscall
        //Return that node
        if(temp->sysCallNum == syscall){
            return temp;
        } else {
            //Check if child
            for(size_t i = 0; i < current->numChildren; i++){
                if(current->children[i]->sysCallNum == syscall){
                    return current->children[i];
                }
            }
        temp = temp->parent;
        }
    }
    //If nothing is found return the same node we started on
    return current;
}


void printGraph(FILE* output, Node* current, int layer, int i){
    // for(int i = 0; i < layer; i++){
    //     fprintf(output, "    ");
    // }

    // fprintf(output, "%d\n", current->sysCallNum);
    if(current == NULL){
        return;
    }
    fprintf(output, " %d [label=\"%d\"];\n", current, current->sysCallNum);
    for(size_t i = 0; i < current->numEdges; i++){
        fprintf(output, "%d -> %d;\n", current->edges[i]->parent, current->edges[i]->child);
    }
    // printf("i: %d: %d\n", i, current->sysCallNum);
    for(size_t i = 0; i < current->numChildren; i++){
        printGraph(output, current->children[i], layer + 1, i+1);
    }
}

bool compareNodes(Node* one, Node* two){
    if(one == NULL || two == NULL){
        return false;
    }

    return(one->sysCallNum == two->sysCallNum && one->parent == two->parent);
}

int main(){

    FILE *fptr;
    FILE *fptr2;
    fptr = fopen("parserOutput.txt", "r");
    fptr2 = fopen("graphOutput.txt", "w");
    char *currentLine;

    Node* root = NULL;
    Node* current = NULL;
    Edge edges[MAX_EDGES];

    if(fptr == NULL || fptr2 == NULL){
        printf("Error, could not open output");
        exit(1);
    }

    currentLine = malloc(MAX_CONTEXT * sizeof(char));
    while(fgets(currentLine, MAX_CONTEXT-1, fptr) != NULL){
        //Temp fix for \n
        char *temp = strstr(currentLine, "\n");
        (*temp) = '\0';

        //Get current num
        int currentSysNum = findNum(currentLine);
        if(currentSysNum == -1){
            printf("%s, %d\n", currentLine, findNum(currentLine));
        } else {

                //Fill root node
                if(root == NULL){
                    root = makeNode(currentSysNum);
                    current = root;
                } else {
                    //Handle way to stop repeated calls.
                    //This should be done before every added node?

                    /*
                    Maybe make way for pairs to be studied?
                    */


                    if(currentSysNum == current->sysCallNum){
                        // printf(" %d\n", compareNodes(setCurrentNode(currentSysNum, current), current));
                        // current = setCurrentNode(currentSysNum, current);
                    } else {
                        //Otherwise we add a new node
                        //And set that to current
                        // setCurrentNode(currentSysNum, current);
                        if(setCurrentNode(currentSysNum, current) != current){
                            current = setCurrentNode(currentSysNum, current);
                        }
                        else {
                            Node* newNode = makeNode(currentSysNum);
                            addNode(current, newNode);
                            current = newNode;
                        }
                    }

                }
        }

    }
    
    //Print graph

    //Start at Root
    //recursive function to call

    //Feed the current node + childNum
    //For current we can get how many children it has
    //For root->numChildren
    //Call print graph
    FILE *dot_file = fopen("graph.dot","w");
    if(!dot_file){
        perror("Failed to open DOT file");
        return 1;
    }
    fprintf(dot_file, "digraph nginx_syscalls {\n");
    printGraph(dot_file, root, 0, 1);
    fprintf(dot_file, "}\n");

}

