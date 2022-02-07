// closh.c - COSC 315, Winter 2020
// Riley Clark
// Reid
// Aalia Omarali

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

#define TRUE 1
#define FALSE 0

// tokenize the command string into arguments - do not modify
void readCmdTokens(char* cmd, char** cmdTokens) {
    cmd[strlen(cmd) - 1] = '\0'; // drop trailing newline
    int i = 0;
    cmdTokens[i] = strtok(cmd, " "); // tokenize on spaces
    while (cmdTokens[i++] && i < sizeof(cmdTokens)) {
        cmdTokens[i] = strtok(NULL, " ");
    }
}

// read one character of input, then discard up to the newline - do not modify
char readChar() {
    char c = getchar();
    while (getchar() != '\n');
    return c;
}

// takes a time in seconds and deletes a processes if it as been running longer than the timeout time
void timeoutHandlerSequential(int timeout, int pid){
    int stat;
    // variables for determining change in time
    time_t before = time(NULL);
    time_t after;
    double difference = 0;
    // let processes run for specified time and check if it has completed
    while(difference <= timeout){
        after = time(NULL);
        // find difference in time
        difference = difftime(after, before);
        // get status of child
        waitpid(pid, &stat, WNOHANG);
        // if child process has completed return back to the program
        if(WIFEXITED(stat)){
            // allow child to terminate
            wait(NULL);
            return; 
        }
    }
    // if time runs out kill process
    printf("Process: %d has been terminated due to a timeout", pid);
    kill(pid, SIGKILL); 
}

// takes a time in seconds and deletes a processes if it as been running longer than the timeout time
void timeoutHandlerParallel(int timeout, int pids[], int count){
    int stat;
    // variables for determining change in time
    time_t before = time(NULL);
    time_t after;
    double difference = 0;
    // keep track of the number of processes which are done
    int done = 0;
    // let processes run for specified time and check if it has completed
    while(difference <= timeout){
        after = time(NULL);
        // find difference in time
        difference = difftime(after, before);
        for(int i = 0; i < count ; i++){
            // if the value is 1000000 then it is complete 
            if(pids[i] == 1000000){
                continue;
            }
            // get status of child
            waitpid(pids[i], &stat, WNOHANG);
            // check if process is complete
            if(WIFEXITED(stat)){
                // let child terminate
                waitpid(pids[i], &stat, 0);
                // set to pid of the child to 1000000 so we can know its done
                pids[i] = 1000000; 
                // if all processes are complete return back to the normal flow
                if(++done != count){
                    sleep(1);
                    return;
                }
            }
        }
    }
    // if timeout oucurs kill processes that are not complete
    for(int i = 0; i < count; i++){
        if(pids[i] == 1000000){
            continue;
        }
        kill(pids[i], SIGKILL); 
        printf("Process: %d has been terminated due to a timeout\n", pids[i]);
    }
}
        

// main method - program entry point
int main() {
    char cmd[81]; // array of chars (a string)
    char* cmdTokens[20]; // array of strings
    int count; // number of times to execute command
    int parallel; // whether to run in parallel or sequentially
    int timeout; // max seconds to run set of commands (parallel) or each command (sequentially)
    
    while (TRUE) { // main shell input loop
        
        // begin parsing code - do not modify
        printf("closh> ");
        fgets(cmd, sizeof(cmd), stdin);
        if (cmd[0] == '\n') continue;
        readCmdTokens(cmd, cmdTokens);
        do {
            printf("  count> ");
            count = readChar() - '0';
        } while (count <= 0 || count > 9);
        
        printf("  [p]arallel or [s]equential> ");
        parallel = (readChar() == 'p') ? TRUE : FALSE;
        do {
            printf("  timeout> ");
            timeout = readChar() - '0';
        } while (timeout < 0 || timeout > 9);
        // end parsing code
        
        
        ////////////////////////////////////////////////////////
        //                                                    //
        // TODO: use cmdTokens, count, parallel, and timeout  //
        // to implement the rest of closh                     //
        //                                                    //
        // /////////////////////////////////////////////////////
        
        
        //array to keep track of forked pids for parrallel proccesses 
        int pids[count];
        //variable to hold pid for sequential proccesses
        int pid;

        if(parallel){
            
            // run the proccess parrallelly 
            for(int i = 0; i < count; i++){
                // fork so child can call execvp
                pids[i] = fork();
                // pid == 0 is the child process
                
                if(pids[i] == 0){
                    // print pid and parent pid
                    printf("I am a child process and my pid is %d, and my parent id is %d\n", getpid(), getppid());
                    // force child to empty buffer
                    fflush(stdout);
                    // excute specified command
                    execvp(cmdTokens[0], cmdTokens);
                    //error statement if command doesnt work 
                    printf("Can't execute %s\n", cmdTokens[0]); // only reached if running the program failed
                    // exit if error ouccurs
                    exit(1);

                // if pid < 0 means fork was unsucessful   
                }else if(pid < 0){
                    printf("Process could not be created");
                    exit(1);
                }
            }
            // call timeout handler
            timeoutHandlerParallel(timeout, pids, count);
            

        }else{
            for(int i = 0; i < count; i++){
                // fork so child can call execvp
                pid = fork();
                
                // pid == 0 is the child process
                if(pid == 0 ){
                    // print pid and parent pid
                    printf("I am a child process and my pid is %d, and my parent id is %d\n", getpid(), getppid());
                    // force child to empty buffer
                    fflush(stdout);
                    // excute specified command
                    execvp(cmdTokens[0], cmdTokens);
                    // error statement if command doesnt work 
                    printf("Can't execute %s\n", cmdTokens[0]); // only reached if running the program failed
                    // exit if error ouccurs
                    exit(1);
                
                // if pid < 0 means fork was unsucessful 
                }else if(pid < 0){
                    printf("Processes could not be created");
                    exit(1);
                
                // parent process will wait til here child process is done
                }else{
                    // wait for child process to finish 
                    timeoutHandlerSequential(pid, timeout);
                }   
            }
        }
        
        // just executes the given command once - REPLACE THIS CODE WITH YOUR OWN
        //execvp(cmdTokens[0], cmdTokens); // replaces the current process with the given program
        // doesn't return unless the calling failed
        //printf("Can't execute %s\n", cmdTokens[0]); // only reached if running the program failed
              
    }
}

