#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#define MAX_LEN 256
#define OS "Windows"

void main(int argc, char *argv[]){
    // Check for valid number of inputs
    if(argc < 2){
        perror("Incorrect number of arguments passed");
        printf("Please input at least one file to be converted from dot to png or a folder");
    }

    // Go through each input
    for(int i = 1; i < argc; i++) {
    
        if(strstr(argv[i], ".dot") == NULL ){ // Does not contain .dot assume directory path
            char *input = argv[i];
            input[strlen(argv[i])-1] = '\0';
            const char *folderName = input;
            DIR *directory = opendir(folderName);  // Open the directory
            // printf("%s\n", folderName);
            if (directory == NULL) {
                perror("Cannot find folder");  // If the directory can't be opened, print an error
            } else {
                char makeDirectory[256];
                sprintf(makeDirectory, " mkdir .\\Graphs\\%s", folderName + strlen(folderName) - 10); // Same identifier
                // printf("%s\n", makeDirectory);
                if (system(makeDirectory) == -1){ // Try the command
                    perror("Could not make subdirectory for graphs");
                }
            }
    
            struct dirent *file;
            while((file = readdir(directory)) != NULL) {
                if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0) {
                    continue;
                } else {
                    char dot[MAX_LEN];
                    char png[MAX_LEN];
    
                    // Build dot file string
                    sprintf(dot, "\"%s\\%s\"", folderName, file->d_name);
                    // printf("Dot file name: %s\n", dot); // Print if neede for debugging
                   
                    sprintf(png, "%.*s.png", strlen(file->d_name)-4, file->d_name);
    
                    char command[MAX_LEN];
                    sprintf(command, "dot -Tpng %s -o %s", dot, png);
                    // printf("%s\n", command);
                    if(system(command) == -1){
                        perror("Could not execute convertion");
                    } else {
                        printf("Converted file %s successfully\n", file->d_name);
                        // Move to graphs folder
                        char moveCommand[MAX_LEN];
                        if(strcmp(OS, "Windows") == 0) { // Is Windows
                            sprintf(moveCommand, "move .\\%s .\\Graphs\\%s", png, folderName + strlen(folderName) - 10);
                            if(system(moveCommand) == -1){
                                perror("Could not execute move command at all");
                            }
                        } else {
                            sprintf(moveCommand, "mv .\\%s .\\Graphs\\%s", png, folderName + strlen(folderName) - 10);
                            if(system(moveCommand) == -1){
                                perror("Could not execute move command at all");
                            }
                        }
                    }                
                }
    
            }
        } else if(strstr(argv[i], ".dot") != NULL ){ //Does contain .dot (we don't want potential fallthrough of bad inputs)
            // PNG name will be same as specified file name
            char png[MAX_LEN];
            char *fileName = strrchr(argv[i], '\\'); // Get last backslash and copy all following content
            if(fileName != NULL) {
                int length =  strlen(argv[i]) - (fileName - argv[i]); // Calculate offset and then total length off file name
                sprintf(png, "%.*s.png", length-5, fileName + 1); // Copy this file name into png
            }
    
            // Build and execute command
            char command[256];
            sprintf(command, "dot -Tpng \"%s\" -o %s", argv[i], png); // Wrap path in ""
    
            // Give user feedback on if command was successful
            if(system(command) == -1){
                perror("Could not execute convertion");
            } else {
                printf("Converted file %s successfully\n", argv[i]);
                char moveCommand[256];
                if(strcmp(OS, "Windows") == 0) { // Is Windows
                    sprintf(moveCommand, "move .\\%s .\\Graphs\\%s", png, png);
                    if(system(moveCommand) == -1){
                        perror("Could not execute move command at all");
                    }
                } else {
                    sprintf(moveCommand, "mv .\\%s .\\Graphs\\%s", png, png);
                    if(system(moveCommand) == -1){
                        perror("Could not execute move command at all");
                    }
                }
            }
        }
    }
}   