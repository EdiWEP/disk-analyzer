#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ftw.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "da_variables.h"

char* statusShm; 
char* progressShm;
int* currentFilesShm;
int* currentDirectoriesShm;

int workerId;
char outputPath[32];

int rootLength;
int totalFiles = 0;
int totalDirectories = 0;
bool countersInitialized = false;

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


int nftwCounter(const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf) {

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

int analyze(const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf) {
    
    if(typeflag == FTW_D) {

        currentSize = 0;
        nftw(fpath, nftwCounter, 1024, 0);

        if(ftwbuf->level == 0){
            totalSize = currentSize;
            countersInitialized = true;
            rootLength = strlen(fpath);
            strcpy(path, fpath);
            *statusShm = 'r';
        }
        else{
            int j = 0;

            for(int i = rootLength; i < strlen(fpath); ++i)
                path[i - rootLength] = fpath[i];
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

        *currentDirectoriesShm += 1;
        outputDirectory(ftwbuf->level);
  
    }
    else if(typeflag == FTW_F){
        *currentFilesShm += 1;
    }

    int progress = 100 * (*currentFilesShm + *currentDirectoriesShm) / (totalFiles + totalDirectories);

    *progressShm = progress;

    return 0;
}


void initialize(char* argv[]) {
    int workerShmFd;
    void* workerData;

    workerId = atoi(argv[2]);

    
    //initialize shared memory
    workerShmFd = shm_open("dskanl_shm", O_RDWR, S_IRUSR | S_IWUSR);
    workerData = mmap(0, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, workerShmFd, 0);
    statusShm = (char*) (workerData + (workerId - 1)*10); 
    progressShm = (char*) (workerData + (workerId - 1)*10 +1);
    currentFilesShm = (int*) (workerData + (workerId - 1)*10 + 2);
    currentDirectoriesShm = (int*) (workerData + (workerId - 1)*10 + 6);
    *currentFilesShm = 0;
    *currentDirectoriesShm = 0;


    //set priority
    int priorityIncrement = 3 - atoi(argv[3]);
    nice(priorityIncrement);

    //initialize file path and output file
    char base[24]; 
    strcpy(base, JOBS_FOLDER_PATH);
    strcat(base, argv[2]);
    strcpy(outputPath, base);
    strcat(outputPath, ".txt");
    
    FILE* fp = fopen(outputPath, "w");
    fclose(fp); 

}

int main(int argc, char* argv[]) {

    if(argc != 4) exit(0);

    initialize(argv);

    nftw(argv[1], analyze, 1024, 0);
    
    *statusShm = 'd';

    return 0;
}