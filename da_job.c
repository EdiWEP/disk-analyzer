#define _XOPEN_SOURCE 500

#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<ftw.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<string.h>
#include<signal.h>
#include<stdbool.h>

int workerId;
int PROGRESS;
int daemonPID;
char STATUS[16];
char outputPath[64];
char statusPath[64];
char daemonPidPath[64];

bool countersInitialized = false;
int rootLength, totalFiles = 0, totalDirectories = 0, doneFiles = 0, doneDirectories = 0;
char type[3];
char path[512];

double percent;
double printSize;
unsigned long totalSize;
unsigned long currentSize;


void outputDirectory(int level){
        int i;

        FILE* fp = fopen(outputPath, "a");

        if(level == 0) fprintf(fp, "Path \t\t Usage \t Size \t Amount\n");
        if(level == 1) fprintf(fp, "|\n");
        if(level >= 1) fprintf(fp ,"|-");

        fprintf(fp, "%s: %0.2f %s, %0.2f%% ", path, printSize, type, percent);

        fprintf(fp, "[");
        for(i = 1; i <= percent / 4; ++i)
            fprintf(fp, "#");

        if(i == 1) { 
            fprintf(fp, "-"); 
        }
        else {
            for(int j = i; j <= 25; ++j) 
                fprintf(fp, "-");
        }
        fprintf(fp, "]");

        fprintf(fp, "\n");

        fclose(fp);
}


int func_count(const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf) {

    if(typeflag == FTW_F)
        currentSize += sb->st_size;

    if(!countersInitialized){
        if(typeflag == FTW_F)
            totalFiles += 1;
        else if(typeflag == FTW_D)
            totalDirectories += 1;
    }

    return 0;
}

int func(const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf) {
    if(typeflag == FTW_D) {
        currentSize = 0;
        nftw(fpath, func_count, 1024, 0);

        if(ftwbuf->level == 0){
            totalSize = currentSize;
            rootLength = strlen(fpath);
            strcpy(path, fpath);
            countersInitialized = true;
        }
        else{
            int j = 0;

            for(int i = rootLength; i < strlen(fpath); ++i){
                path[i - rootLength] = fpath[i];
            }

            path[strlen(fpath) - rootLength] = '\0';
        }

        percent = (double) currentSize / totalSize * 100;

        if(currentSize < 1024){
            strcpy(type, "B");
            printSize = currentSize;
        }
        else if(currentSize < 1048576){
            strcpy(type, "KB");
            printSize = (double) currentSize / 1024;
        }
        else if(currentSize < 1073741824){
            strcpy(type, "MB");
            printSize = (double) currentSize / 1048576;
        }
        else{
            strcpy(type, "GB");
            printSize = (double) currentSize / 1073741824;
        }

        doneDirectories +=1;
        outputDirectory(ftwbuf->level);
    }
    else if(typeflag == FTW_F){
        doneFiles += 1;
    }
    return 0;
}

void SendStatus(int sig) {

    PROGRESS = 100 * (doneFiles + doneDirectories) / (totalFiles + totalDirectories);
    FILE* fp = fopen(statusPath, "w");
    fprintf(fp, "%s\n, %d", STATUS, PROGRESS);
    fclose(fp);

    kill(daemonPID, SIGUSR2);
}

void initialize(char* argv[]) {

    strcpy(STATUS, "PREPARING");
    signal(SIGUSR2, SendStatus);
    workerId = atoi(argv[2]);
    
    char base[24]; 
    strcpy(base, "/tmp/dad/jobs/");
    strcat(base, argv[2]);
    strcpy(outputPath, base);
    strcat(outputPath, ".txt");

    strcpy(base, "/tmp/dad/status/");
    strcat(base, argv[2]);
    strcpy(statusPath, base);
    strcat(statusPath, ".txt");

    strcpy(daemonPidPath, "/tmp/dad/daemon_pid.txt");

    FILE* fp = fopen(daemonPidPath, "r");
    char* pid = malloc(10);
    fread(pid, 10, 1, fp);
    daemonPID = atoi(pid);
    free(pid);
    fclose(fp);
    
    fp = fopen(outputPath, "w");
    fclose(fp);
}

int main(int argc, char* argv[]) {

    if(argc != 3) exit(0);

    initialize(argv);

    strcpy(STATUS, "IN PROGRESS");
    nftw(argv[1], func, 1024, 0);
    strcpy(STATUS, "DONE");

    return 0;
}