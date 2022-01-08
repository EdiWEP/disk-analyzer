#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

#include "da_variables.h"

#define MAX_TASKS 10

int activeWorkers = 0;
int active[MAX_TASKS+1];


FILE* logfp;
int workerShmFd;
void* workerData;
// 1b[status]1b[percent]4b[doneFiles]4b[totalDirs] | next job | ...

char* paths[MAX_TASKS+1];
char** status; 
char** progress;
int** doneFiles;
int** doneDirectories;
// job statuses
// i -> preparing / initialization stage
// r -> in progress / running
// d -> done
// s -> suspended

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

    fprintf(fp, "%d\n", getpid());

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

    munmap(workerData, getpagesize());
    close(workerShmFd);
    shm_unlink(WORKER_SHM_NAME);

    exit(0);
}

void setupSharedMem() {
    workerShmFd = shm_open(WORKER_SHM_NAME, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);

    if (workerShmFd < 0) {
        fprintf(stderr, "Error: Failed to open shared memory");
        stop(0);
    }

    if(ftruncate(workerShmFd, getpagesize()) == -1) {
        fprintf(stderr, "Error: Failed to resize shared memory");
        stop(0); 
    } 

    void* workerData = mmap(0, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, workerShmFd, 0);

    if(workerData == MAP_FAILED){
        fprintf(stderr, "Error: Failed to get memory map of worker data");
        stop(0);
    }

    status = malloc(sizeof(char*) * (MAX_TASKS+1));
    progress = malloc(sizeof(char*) * (MAX_TASKS+1));
    doneFiles = malloc(sizeof(int*) * (MAX_TASKS+1));
    doneDirectories = malloc(sizeof(char*) * (MAX_TASKS+1)); 


    for(int i = 1; i <= MAX_TASKS; ++i) {
        status[i] = (char*) (workerData + (i-1)*10);
        progress[i] = (char*) (workerData + (i-1)*10 +1);
        doneFiles[i] = (int*) (workerData + (i-1)*10 + 2);
        doneDirectories[i] = (int*) (workerData + (i-1)*10 + 6);
         
    }
}



int firstFreeId(){

    for(int i = 1; i <= MAX_TASKS; ++i) {
        if(!active[i]) {
            return i; 
        }
    }

    return -1;
}

void commandHandler(int signal) {

    FILE* fpi = fopen(INSTRUCTION_PATH, "r");
    
    int code;
    fscanf(fpi, "%d", &code);
    
    char id[3];
    char path[512];
    int callerPid;

    switch(code) {

        case ADD: 

            FILE* outfp = fopen(DAEMON_OUTPUT_PATH, "w");

            char prio[3];
            int pr;

            int newId = firstFreeId();

            sprintf(id, "%d", newId);
            fscanf(fpi, "%d", &pr);
            sprintf(prio, "%d", pr);
            fscanf(fpi, "%s", path);
            fscanf(fpi, "%d", &callerPid);

            if(newId == -1) {
                fprintf(outfp, "Error: Couldn't start new job.\nReason: Maximum amount(%d) of jobs reached.\nUse -r to remove jobs.", MAX_TASKS);
            
                fclose(outfp);
                kill(callerPid, SIGUSR1);
                break;
            }

            ++activeWorkers;
            active[newId] = 1;
            *status[newId] = 'i';
            *progress[newId] = 0;
            *doneFiles[newId] = 0;
            *doneDirectories[newId] = 0;     
			
			strcpy(paths[newId], path);
			
            char* argv[] = {"diskanalyzer_job", path, id, prio, NULL};
            int pid = fork();

            if(pid == 0) {
                execve(DISKANALYZER_JOB_PATH, argv, NULL);
            }


            fprintf(outfp, "0\nStarted new analysis job\nID = %d\nDirectory = %s\n", newId, path);
            
            fclose(outfp);
            kill(callerPid, SIGUSR1); 

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
    setupSharedMem();
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


