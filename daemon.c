#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <fcntl.h>

#include "da_variables.h"

FILE* logfp;


void becomeDaemon() {
    pid_t pid;

    // fork off the parent process
    pid = fork();

    // error check
    if (pid < 0)
        exit(EXIT_FAILURE);

    // success: Let the parent terminate
    if (pid > 0)
        exit(EXIT_SUCCESS);

    // on success: The child process becomes session leader
    if (setsid() < 0)
        exit(EXIT_FAILURE);


    // fork off for the second time
    pid = fork();

    // an error occurred
    if (pid < 0)
        exit(EXIT_FAILURE);

    // success: Let the parent terminate
    if (pid > 0)
        exit(EXIT_SUCCESS);

    // set new file permissions
    umask(0);

    // change the working directory to the root directory
    chdir("/");

    // close all open file descriptors
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x >= 0; x--) {
        close (x);
    }
    
    
}

void makeDirectories() {

    struct stat st;    
    if (stat("/tmp/dad", &st) == -1) {
        mkdir("/tmp/dad", 0700);
        mkdir("/tmp/dad/jobs", 0700);
        mkdir("/tmp/dad/status", 0700);
    }

}

void printPid() {
    FILE* fp = fopen(DAEMON_PID_PATH, "w");

    fprintf(fp, "%d", getpid());

    fclose(fp);
}

void openLogs() {

    int errfd = open(ERROR_LOG_PATH, O_RDWR | O_CREAT | O_APPEND | O_TRUNC, 0600);
    dup2(errfd, fileno(stderr));
    close(errfd);

    logfp = fopen(LOG_PATH, "w");
}

void stop(int signal) {

    fclose(logfp);
    exit(0);
}


void commandHandler(int signal) {

    FILE* fpi = fopen(INSTRUCTION_PATH, "r");
    
    int code;
    fscanf(fpi, "%d", &code);
    
    switch(code) {

        case ADD: 

        break;

        case SUSPEND: 

        break;
        
        case RESUME: 

        break;
        
        case REMOVE:

        break;

        case INFO: 

        break;

        case PRINT: 

        break;
        
        case LIST_ALL: 

        break;
        
        default: 
            fprintf(stderr,"Error: received unknown command code\n"); 
        break;
    } 

    fclose(fp);
    fclose(fpi);
}


void setupSignals() {

    signal(SIGTERM, stop);   
    signal(SIGUSR1, commandHandler);
}


void initialize() {
    
    becomeDaemon();
    openLogs();
    setupSignals();
    makeDirectories();
    printPid();
}



void run() {

    while(1) {
        
        sleep(1);
    }
}

int main()
{
    initialize();

    run();

    return 0;
}

