#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <error.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/stat.h>

#include "da_variables.h"


int daemonPID;
int processPID;
char instruction[1024];


int getDaemonPid()
{
    int d_pid;
    FILE* fptr = fopen(DAEMON_PID_PATH, "r");
    if(fptr == NULL){
        fprintf(stderr,"Error: Couldn't open the Daemon PID file\n");
        return -1;
    }
    fscanf(fptr, "%d", &d_pid);
    fclose(fptr);
    return d_pid;
}

struct stat sts;
int getDaemonExistence(){
    char daemon[32];

    int pid = getDaemonPid();
    if(pid == -1){
        //proces doesnt exist
        return 0;
    }

    sprintf(daemon,"/proc/%d",pid); 

    if(stat(daemon,&sts)== -1 && errno == ENOENT){
        //process doesnt exist
        return 0;
    }
    //process exists
    return 1;
}

int startDaemon(){
    char *argv[] = {"./dad",NULL};

    if(getDaemonExistence() == 0){
        //launch daemon
        int pid = fork();

        if(pid == 0){
            if(execvp("./dad",argv) == -1){
                perror(NULL);
                return errno;
            };
        }
        printf("Daemon Launched\n");
    }
    else {
        printf("Daemon is already running\n");
    }
    return 0;
}

int getOption(char* opt, int argc)
{
    if( strcmp(opt,"-a") == 0 || strcmp(opt,"--add") == 0 )  return ADD;
    else if( ( strcmp(opt,"-p") == 0 || strcmp(opt,"--priority") == 0 ) && argc == 5) return PRIORITY;
    else if( strcmp(opt,"-S") == 0 || strcmp(opt,"--suspend") == 0 ) return SUSPEND;
    else if( strcmp(opt,"-R") == 0 || strcmp(opt,"--resume") == 0 ) return RESUME;
    else if( strcmp(opt,"-r") == 0 || strcmp(opt,"--remove") == 0 ) return REMOVE;
    else if( strcmp(opt,"-i") == 0 || strcmp(opt,"--info") == 0 ) return INFO;
    else if( strcmp(opt,"-p") == 0 || strcmp(opt,"--print") == 0) return PRINT;
    else if( strcmp(opt,"-l") == 0 || strcmp(opt,"--list") == 0) return LIST_ALL;
    else if( strcmp(opt, "start") == 0) return START;
    else if (strcmp(opt,"-h") == 0 || strcmp(opt,"--help") == 0) return HELP;
    return -1;
}

int sendCommand() {

    FILE* fp = fopen(INSTRUCTION_PATH, "w");
    if(fp == NULL){
        fprintf(stderr, "Error: Could not open Instruction file\n");
        exit(-1);
    }
    fprintf(fp, instruction, NULL);
    fclose(fp);

    int result = kill(daemonPID, SIGUSR1);
    
    if (result) {
        fprintf(stderr, "Error: Failed to send signal to daemon\n");
        exit(-1);
    }

    sleep(3); // wait for daemon signal;

    return 0;
}

void getResponse(int signal) {

    int responseCode;
    bool readJob = false;
    char line[1024];

    FILE* fp = fopen(DAEMON_OUTPUT_PATH, "r");
    fscanf(fp, "%d", &responseCode);

    if(responseCode > 0) {
        readJob = true;
    }

    if(readJob) {
        fclose(fp);
        char outputPath[32];
        char code[5];
        sprintf(code, "%d", responseCode);

        strcpy(outputPath, JOBS_FOLDER_PATH);
        strcat(outputPath, code);
        strcat(outputPath, ".txt");

        fp = fopen(outputPath, "r");

        while(fread(line, 1, 1024, fp)) {
            printf("%s\n", line);
        }
    }
    else {
        while(fread(line, 1, 1024, fp)) {
            printf("%s\n", line);
        }
    }

    exit(0);

}

void initialize() {
    daemonPID = getDaemonPid();
    processPID = getpid();
    signal(SIGUSR1, getResponse);
}

int main(int argc, char *argv[])
{
    if(argc == 1) {
        fprintf(stderr, "Error: No arguments given.\nUse da -h for usage details\n");
        return -1;
    }
    if(argc == 4 || argc > 5) {
        fprintf(stderr, "Error: Unknown command.\nUse da -h for usage details\n");
    }

    int id;

    int option = getOption(argv[1], argc);

    if(option == START){
        if(startDaemon() != 0){
            perror(NULL);
            return errno;
        }
        return 0;
    }


    initialize();

    switch (option)
    {
        case ADD:
            
            if(argc < 3){
                fprintf(stderr, "Error: Not enough arguments\n");
                return -1;
            }

            int priority = 1;
            char* dirPath = argv[2];
            
            if(argc == 5 && getOption(argv[3], argc) == PRIORITY){

                priority = atoi(argv[4]);
                if(priority > 3) priority = 3;
                else if (priority < 1) priority = 1;

            }

            sprintf(instruction,"%d\n%d\n%s\n%d\n", ADD, priority, dirPath, processPID);
            break;

        case SUSPEND:
            if(argc != 3){
                fprintf(stderr, "Error: Wrong number of arguments\n");
                return -1;
            }

            id = atoi(argv[2]);
            sprintf(instruction,"%d\n%d\n%d\n", SUSPEND, id, processPID);
            break;

        case RESUME:
            if(argc != 3){
                fprintf(stderr, "Error: Wrong number of arguments\n");
                return -1;
            }

            id = atoi(argv[2]);
            sprintf(instruction,"%d\n%d\n%d\n", RESUME, id, processPID);
            break;
        
        case REMOVE:
            if(argc!=3){
                fprintf(stderr, "Error: Wrong number of arguments\n");
                return -1;
            }

            id = atoi(argv[2]);
            sprintf(instruction,"%d\n%d\n%d\n", REMOVE, id, processPID);
            break;
        
        case INFO:
            if(argc!=3){
                fprintf(stderr, "Error: Wrong number of arguments\n");
                return -1;
            }

            id = atoi(argv[2]);
            sprintf(instruction,"%d\n%d\n%d\n", INFO, id, processPID);
            break;
        
        case PRINT:
            if(argc!=3){
                fprintf(stderr, "Error: Wrong number of arguments\n");
                return -1;
            }

            id = atoi(argv[2]);
            sprintf(instruction,"%d\n%d\n%d\n", PRINT, id, processPID);
            break;
        
        case LIST_ALL:
            if(argc != 2){
                fprintf(stderr, "Error: Wrong number of arguments\n");
                return -1;
            }

            sprintf(instruction,"%d\n%d\n", LIST_ALL, processPID);
            break;

        default:
            fprintf(stderr, "Error: Invalid instruction\n");
            option = HELP;
        case HELP:
            printf(
                "Usage: da [OPTION]... [DIR]...\n"
                "Analyze the space occupied by the directory at [DIR]\n"
                "Use 'da start' to launch the Disk Analyzer Daemon\n\n"
                "-a, --add analyze a new directory path for disk usage\n"
                "-p, --priority set priority for the new analysis (works only with -a argument)\n"
                "-S, --suspend <id> suspend task with <id>\n"
                "-R, --resume <id> resume task with <id>\n"
                "-r, --remove <id> remove the analysis with the given <id>\n"
                "-i, --info <id> print status about the analysis with <id> (pending, progress, done)\n"
                "-l, --list list all analysis tasks, with their ID and the corresponding root path\n"
                "-p, --print <id> print analysis report for those tasks that are done\n"
            );
            break;
        }
    if(option != HELP)
        sendCommand();

    return 0;
}