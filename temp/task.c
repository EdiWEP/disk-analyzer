#define _XOPEN_SOURCE 500
 
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<ftw.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<string.h>
 
int rootLength;
char type[3];
char path[512];
 
double percent;
double printSize;
unsigned long totalSize;
unsigned long currentSize;
 
 
void outputDirectory(int level){
        int i;
 
        if(level == 1) printf("|\n");
        if(level >= 1) printf("|-");
 
        printf("%s: %0.2f %s, %0.2f%% ", path, printSize, type, percent);
 
        printf("[");
        for(i = 1; i <= percent / 4; ++i)
            printf("#");
 
        if(i == 1) { 
            printf("-"); 
        }
        else {
            for(int j = i; j <= 25; ++j) 
                printf("-");
        }
        printf("]");
 
        printf("\n");
 
}
 
 
int func_count(const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf) {
 
    if(typeflag == FTW_F)
        currentSize += sb->st_size;
 
    return 0;
}
 
int func(const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf) {
    if(typeflag == FTW_D) {
        currentSize = 0;
        nftw(fpath, func_count, 1024, 0);
 
        if(ftwbuf->level == 0){
            printf("Path \t\t Usage \t Size \t Amount\n");
            totalSize = currentSize;
            rootLength = strlen(fpath);
            strcpy(path, fpath);
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
 
        outputDirectory(ftwbuf->level);
 
    }
    return 0;
}
 
 
int main() {
 
    nftw("/home/kali/Desktop/Laboratoare/", func, 1024, 0);
 
    return 0;
}