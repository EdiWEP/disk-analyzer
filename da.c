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
#include <dirent.h>
#include "da_variables.h"


int daemonPID;
int processPID;
char instruction[1024];


int getDaemonPid()
{
    int d_pid;
    FILE* fptr = fopen(DAEMON_PID_PATH, "r");
    if(fptr == NULL){
        fprintf(stderr,"Error: Couldn't open the Daemon PID file. Check if daemon is running\nUse da -h for usage details\n");
        return -1;
    }
    fscanf(fptr, "%d", &d_pid);
    fclose(fptr);
    return d_pid;
}

int getDaemonExistence(){
    char daemon[32];
    struct stat sts;
    if(stat(DAEMON_PID_PATH, &sts) == -1 && errno == ENOENT) {
        return 0;
    }  
    
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

void startDaemon(){
    char *argv[] = {"dad",NULL};

    if(getDaemonExistence() == 0){
        //launch daemon
        int pid = fork();

        if(pid == 0){
            if(execvp(DAEMON_BINARY_PATH, argv) == -1){
                fprintf(stderr, "Error: Daemon failed to start, errno: %d\n", errno);
                exit(0);
            };
        }
        printf("Daemon Launched\n");
    }
    else {
        printf("Daemon is already running\n");
    }
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
        fprintf(stderr, "Error: Failed to send signal to daemon. Check if daemon is running\nUse da -h for usage details\n\n");
        exit(-1);
    }

    sleep(3); // wait for daemon signal;

    return 0;
}

void getResponse(int signal) {

    int responseCode;
    char c; 

    FILE* fp = fopen(DAEMON_OUTPUT_PATH, "r");
    fscanf(fp, "%d", &responseCode);
    c = fgetc(fp); // skip first \n

    if(responseCode > 0) {
        fclose(fp);
        char outputPath[32];
        char code[5];
        sprintf(code, "%d", responseCode);

        strcpy(outputPath, JOBS_FOLDER_PATH);
        strcat(outputPath, code);
        strcat(outputPath, ".txt");

        fp = fopen(outputPath, "r");

        while(( c = fgetc(fp)) != EOF) {
            printf("%c",c);
        }
        
    }
    else {
        while((c=fgetc(fp)) != EOF) {
            printf("%c",c);
        }
        
    }

    fclose(fp);
    
    exit(0);

}

void initialize() {
    daemonPID = getDaemonPid();
    processPID = getpid();
    signal(SIGUSR1, getResponse);
}

//returns 0 if argument is valid, -1 otherwise
bool getNumericArgument(char *argv[], int *variable, int index){

    char * inputcheck;

    *variable = (int) strtol(argv[index], &inputcheck, 10);
                
    //check if argument given is a number
    if(*inputcheck != '\0'){
        if(index == 4)
            fprintf(stderr, "Error: Priority must be numeric\n");
        else if(index == 2)
            fprintf(stderr, "Error: Identifier must be numeric\n");
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if(argc == 1) {
        fprintf(stderr, "Error: No arguments given\nUse da -h for usage details\n");
        return -1;
    }
    if(argc == 4 || argc > 5) {
        fprintf(stderr, "Error: Unknown command\nUse da -h for usage details\n");
    }

    int id;

    int option = getOption(argv[1], argc);

    if(option == START){
        startDaemon();
        return 0;
    }

    initialize();

    switch (option)
    {
        case ADD:
            
            int priority = 1;
            char* dirPath = argv[2];

            //check if path was given
            if(argc == 2){
                fprintf(stderr, "Error: -a requires an argument(path)\n");
                return -1;
            }

            //check if path is valid
            DIR* checkDirectory = opendir(dirPath);
            if(ENOENT == errno){
                fprintf(stderr, "Error: Invalid or inexistent path\n");
                return -1;
            }
            closedir(checkDirectory);

            //if priority exists, get data
            if(argc == 5 && getOption(argv[3], argc) == PRIORITY){

                //get priority argument and check if it is numeric
                if(getNumericArgument(argv, &priority, 4))
                    return -1;

                if(priority > 3){ 
                    priority = 3;
                    printf("Warning: Priority was set to maximum of 3\n");
                }
                else if (priority < 1){ 
                    priority = 1;
                    printf("Warning: Priority was set to minimum of 1\n");
                    }
            }

            sprintf(instruction,"%d\n%d\n%s\n%d\n", ADD, priority, dirPath, processPID);
            break;

        case SUSPEND:
            if(argc != 3) {
                fprintf(stderr, "Error: Wrong number of arguments\nUsage: da [OPTION] [ID]\n");
                return -1;
            }

            //get identifier argument and check if it is numeric
            if(getNumericArgument(argv, &id, 2))
                    return -1;

            sprintf(instruction,"%d\n%d\n%d\n", SUSPEND, id, processPID);
            break;

        case RESUME:
            if(argc != 3) {
                fprintf(stderr, "Error: Wrong number of arguments\nUsage: da [OPTION] [ID]\n");
                return -1;
            }

           //get identifier argument and check if it is numeric
            if(getNumericArgument(argv, &id, 2))
                    return -1;

            sprintf(instruction,"%d\n%d\n%d\n", RESUME, id, processPID);
            break;
        
        case REMOVE:
            if(argc != 3) {
                fprintf(stderr, "Error: Wrong number of arguments\nUsage: da [OPTION] [ID]\n");
                return -1;
            }

            //get identifier argument and check if it is numeric
            if(getNumericArgument(argv, &id, 2))
                    return -1;

            sprintf(instruction,"%d\n%d\n%d\n", REMOVE, id, processPID);
            break;
        
        case INFO:
            if(argc != 3) {
                fprintf(stderr, "Error: Wrong number of arguments\nUsage: da [OPTION] [ID]\n");
                return -1;
            }

            //get identifier argument and check if it is numeric
            if(getNumericArgument(argv, &id, 2))
                    return -1;

            sprintf(instruction,"%d\n%d\n%d\n", INFO, id, processPID);
            break;
        
        case PRINT:
            if(argc != 3) {
                fprintf(stderr, "Error: Wrong number of arguments\nUsage: da [OPTION] [ID]\n");
                return -1;
            }
            
            //get identifier argument and check if it is numeric
            if(getNumericArgument(argv, &id, 2))
                    return -1;

            sprintf(instruction,"%d\n%d\n%d\n", PRINT, id, processPID);
            break;
        
        case LIST_ALL:

            sprintf(instruction,"%d\n%d\n", LIST_ALL, processPID);
            break;

        default:
            fprintf(stderr, "Error: Invalid instruction\n");
            option = HELP;
        case HELP:
            printf(
                "DiskAnalyzer - a daemon that performs and manages disk space analysis jobs\n\n"
                "USAGE\n\n"
                "  da start                Launch the daemon. Do this before trying to start a job\n"
                "  da -a [DIR]             Perform analysis on directory at [DIR]. Absolute path only\n"
                "  da -a [DIR] -p [1-3]    Set priority of analysis after starting. 3 is highest\n"
                "  da -l                   List job statuses, including ID and priority\n"
                "  da [OPTION] [ID]        Send command [OPTION] regarding job with [ID]. [OPTION] must be a flag \n"
                "\nFLAGS\n\n"
                "  -a, --add [DIRPATH]     Start new analysis job on directory [DIRPATH]\n"
                "  -p, --priority [1-3]    Set job priority (only after -a flag)\n"
                "  -l, --list              Srint status of all jobs\n"
                "  -i, --info [id]         Print status of job with the given [id]\n"
                "  -p, --print [id]        Print analysis results of job with the given [id]\n"
                "  -r, --remove [id]       Remove task with the given [id]\n"
                "  -S, --suspend [id]      Suspend task with the given [id]\n"
                "  -R, --resume [id]       Resume task with the given [id]\n"
            );
            break;
        }
    if(option != HELP) 
        sendCommand();

    return 0;
}