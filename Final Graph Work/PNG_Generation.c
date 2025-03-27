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
        char dot[argLen+5];
        char png[argLen+5];

        // Fill dot and png file details
        strncpy(dot, argv[i], argLen);
        strncpy(png, argv[i], argLen);
        strncat(dot, ".dot", argLen+5);
        strncat(png, ".png", argLen+5);

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
        sprintf(moveCommand, "mv %s ./Graphs", png);
        // Give user feedback on if command was successful
        if(system(moveCommand) == -1){
            perror("Could not execute move");
        } else {
            printf("Moved file %s  to 'Graphs' successfully\n", argv[i]);
        }
    }
}