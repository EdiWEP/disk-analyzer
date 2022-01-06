#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <error.h>
#include <stdlib.h>
#include <stdarg.h>
#include "da_variables.h"

int get_daemon_pid()
{
    int d_pid;
    FILE* fptr = fopen(DAEMON_PID_PATH,"r");
    if(fptr == NULL){
        printf("Error: Could not open Daemon Pid file");
        return -1;
    }
    fscanf(fptr,"%d",&d_pid);
    fclose(fptr);
    return d_pid;
}

int write_in_file(const char* data)
{
    FILE* fptr = fopen(INSTRUCTION_PATH,"w");
    if(fptr == NULL){
        printf("Error: Could not open Instruction Path file");
        return -1;
    }
    fprintf(fptr,data,NULL);
}

int get_option(char* opt)
{
    if( strcmp(opt,"-a") == 0 || strcmp(opt,"--add") == 0 )  return ADD;
    else if( strcmp(opt,"-p") == 0 || strcmp(opt,"--priority") == 0 ) return PRIORITY;
    else if( strcmp(opt,"-S") == 0 || strcmp(opt,"--suspend") == 0 ) return SUSPEND;
    else if( strcmp(opt,"-R") == 0 || strcmp(opt,"--resume") == 0 ) return RESUME;
    else if( strcmp(opt,"-r") == 0 || strcmp(opt,"--remove") == 0 ) return REMOVE;
    else if( strcmp(opt,"-i") == 0 || strcmp(opt,"--info") == 0 ) return INFO;
    else if( strcmp(opt,"-p") == 0 || strcmp(opt,"--print") == 0) return PRINT;
    else if( strcmp(opt,"-l") == 0 || strcmp(opt,"--list") == 0) return LIST_ALL;
    else if (strcmp(opt,"-h") == 0 || strcmp(opt,"--help") == 0) return HELP;
    return -1;
}

int main(int argc, char **argv)
{
    if(argc == 1){
        printf("No Arguments\n");
        return -1;
    }

    pid_t daemon_pid = get_daemon_pid();
    pid_t process_pid = getpid();
    int id;
    char instruction[1000];

    int option = get_option(argv[1]);

    switch (option)
    {
        case ADD:
            
            if(argc < 3){
                printf("Not enough arguments.");
                return -1;
            }

            int priority = 1;
            char* dirPath = argv[2];
            
            if(argc == 5 && get_option(argv[3]) == PRIORITY){

                priority = atoi(argv[4]);
                if(priority > 3) priority = 3;
                else if (priority < 1) priority = 1;

            }

            sprintf(instruction,"Instruction - %d\nPriority - %d\nPath - %s\nDA_Pid - %d\n",ADD,priority,dirPath,getpid());
            break;

        case SUSPEND:
            if(argc != 3){
                printf("Wrong number of arguments.");
                return -1;
            }

            id = atoi(argv[2]);
            sprintf(instruction,"Instruction - %d\nId - %d\nDA_Pid - %d\n",SUSPEND,id,getpid());
            break;

        case RESUME:
            if(argc != 3){
                printf("Wrong number of arguments.");
                return -1;
            }

            id = atoi(argv[2]);
            sprintf(instruction,"Instruction - %d\nId - %d\nDA_Pid - %d\n",RESUME,id,getpid());
            break;
        
        case REMOVE:
            if(argc!=3){
                printf("Wrong number of arguments.");
                return -1;
            }

            id = atoi(argv[2]);
            sprintf(instruction,"Instruction - %d\nId - %d\nDA_Pid - %d\n",REMOVE,id,getpid());
            break;
        
        case INFO:
            if(argc!=3){
                printf("Wrong number of arguments.");
                return -1;
            }

            id = atoi(argv[2]);
            sprintf(instruction,"Instruction - %d\nId - %d\nDA_Pid - %d\n",INFO,id,getpid());
            break;
        
        case PRINT:
            if(argc!=3){
                printf("Wrong number of arguments.");
                return -1;
            }

            id = atoi(argv[2]);
            sprintf(instruction,"Instruction - %d\nId - %d\nDA_Pid - %d\n",PRINT,id,getpid());
            break;
        
        case LIST_ALL:
            if(argc != 2){
                printf("Wrong number of arguments.");
                return -1;
            }

            sprintf(instruction,"Instruction - %d\nDA_Pid - %d\n",LIST_ALL,getpid());
            break;

        case HELP:

            printf(
                "Usage: da [OPTION]... [DIR]...\n"
                "Analyze the space occupied by the directory at [DIR]\n\n"
                "-a, --add analyze a new directory path for disk usage\n"
                "-p, --priority set priority for the new analysis (works only with -a argument)\n"
                "-S, --suspend <id> suspend task with <id>\n"
                "-R, --resume <id> resume task with <id>\n"
                "-r, --remove <id> remove the analysis with the given <id>\n"
                "-i, --info <id> print status about the analysis with <id> (pending, progress, done)\n"
                "-l, --list list all analysis tasks, with their ID and the corresponding root path\n"
                "-p, --print <id> print analysis report for those tasks that are done\n"
            );

        default:
            printf("Invalid instruction");
        }
    
    write_in_file(instruction);

    return 0;
}