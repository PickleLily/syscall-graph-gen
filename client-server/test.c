#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char* argv[]){
	
	pid_t pid = getpid();
	printf("PID: %d\n", pid);
	printf("process_name: %s\n", argv[0]);
    if (argc < 2){
        printf("did not specify enough arguments...\n");
        return -1;
    }
    int arg = atoi(argv[1]);
    
    if(arg == 1){
        printf("specified option 1 -- open, read and write to a file\n");

        // open file
        FILE *f = fopen("./file.txt", "r+");
        // read contents
        if(f == NULL){
            printf("file open failure...\n");
            return 0;
        }
            char buff1[10];
            fgets(buff1, sizeof(buff1),f);
            fseek(f, 0, SEEK_SET);

            int n = atoi(buff1);
            printf("number: %d \n", n);
            int new_n = n + 1;
            char int_ptr[10];

            sprintf(int_ptr, "%d", new_n);

            fprintf(f, "%d", new_n);
            fseek(f, 0, SEEK_SET);

            char buff[10];
            fgets(buff, sizeof(buff),f);
            fseek(f, 0, SEEK_SET);

            int val = atoi(buff);
            printf("new number: %d \n", val);

            fclose(f);
    
    }else if(arg == 2){
        printf("specified option 2 -- make child process\n");

        pid_t process;
        process = fork();

        if(process < 0){
            printf("fork failed\n");
        }else if(process > 0){
            printf("Parent\n");
        }else{
            printf("Child\n");
        }
    }
    else if(arg == 3){
        printf("specified option 3 -- get current time\n");
        time_t t;
        time(&t);
        printf("current time:%s", ctime(&t));
        return 0;  
    }else{
        printf("Specified an option that was not available\n");
        return -1;
    }  
}
