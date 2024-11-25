#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_CHAR_LENGTH 56
#define SYSCALL_LEN 20

void main(){
    //Read from Falco output file
    FILE *fptr;
    FILE *fptr2;
    char currentLine[MAX_CHAR_LENGTH];
    char *pastCall;
    fptr = fopen("falcoOutput.txt", "r");
    fptr2 = fopen("parserOutput.txt", "w");

    //Test to open file
    if(fptr == NULL || fptr2 == NULL){
        printf("Error, could not open output");
        exit(1);
    }

    //Get all lines
    while(fgets(currentLine, MAX_CHAR_LENGTH-1, fptr) != NULL){
        /*Parse the inputs of all lines
        strstr gets the char index of where the specified string occurs*/
        char *notice = strstr(currentLine, "Notice");
        char *by = strstr(currentLine, " by");

        //Make sure we found both strings
        if(notice != NULL && by != NULL){
            //Edit so that we skip past word Notice
            char *postNotice = notice + strlen("Notice ");
            //Terminate string after the word by (past event)
            *(by) = '\0';

            //If it is not a kernal event
            if(strcmp(postNotice, "<NA>") != 0){
                //Weird prlimit case:
                if(strcmp(postNotice, "prlimit") == 0){
                    char *temp = "64";
                    strcat(postNotice, temp);
                }
                //If nothing exists in pastCall fill
                if(pastCall == NULL){
                    // printf("%s\n", postNotice); //Immitate output to file
                    pastCall = malloc(SYSCALL_LEN * sizeof(char));
                    strncpy(pastCall, postNotice, strlen(postNotice));
                    fprintf(fptr2, "%s\n", postNotice);
                //If something does exist
                } else {
                    //Is it NOT the same as what we're currently looking at
                    if(strcmp(pastCall, postNotice) != 0){
                        // printf("%s\n", postNotice); //Immitate output to file
                        free(pastCall);
                        pastCall = malloc(SYSCALL_LEN * sizeof(char));
                        strncpy(pastCall, postNotice, strlen(postNotice));
                        fprintf(fptr2, "%s\n", postNotice);
                    }
                }
            }
        }
    }
    //Close our files
    fclose(fptr);
    fclose(fptr2);
}

