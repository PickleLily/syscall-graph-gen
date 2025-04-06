#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 100

int main(int argc, char *argv[]){
    if(argc < 2){
        perror("Incorrect number of arguments passed");
        printf("Please input at least 1 file to be converted from dot to png");
    }

    for(int i = 1; i < argc; i++){
        // Build the dot and png file for each
        int argLen = strnlen(argv[i], MAX_LEN)+1; // Have to re-add NULL terminator
        char dot[argLen+4+14+4];


        // Build dot file string
        strncpy(dot, "\"", 2); // Place starting "
        strncat(dot, ".\\Dot Files\\", 14); // Add file path name
        strncat(dot, argv[i], argLen); // Add actual file name
        strncat(dot, ".dot", argLen+4); // Add .dot to end
        strncat(dot, "\"", 2); // Add closing "
        printf("%s\n", dot); // Print if neede dfor debugging

        // Build png file name
        char png[argLen+5];
        strncpy(png, argv[i], argLen);
        strncat(png, ".png", argLen+4);

        // Build and execute command
        char command[256];
        sprintf(command, "dot -Tpng %s -o %s", dot, png);

        // Give user feedback on if command was successful
        if(system(command) == -1){
            perror("Could not execute convertion");
        } else {
            printf("Converted file %s successfully\n", argv[i]);
        }

        // Move to graphs folder
        char moveCommand[256];
        char otherMoveCommand[256];
        sprintf(moveCommand, "mv .\\%s ./Graphs", png);
        // Give user feedback on if command was successful
        if(system(moveCommand) == -1){
            // Try Windows version
            sprintf(otherMoveCommand, "move %s ./Graphs", png);
            if(system(otherMoveCommand) == -1){
                perror("Could not execute move");
            }
        } else {
            printf("Moved file %s  to 'Graphs' successfully\n", argv[i]);
        }
    }
}