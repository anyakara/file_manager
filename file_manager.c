#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "FCB.h"
#include <sys/qos.h>
#include<sys/stat.h>
#include<unistd.h>
#include<dirent.h>

/* File manager that mimics FCB functionality without relying
on dos.h requires simulating the operations of FCBS in a
modern environment, involving the creation of own structures
and functions to handle files, similar to how FCBs work in DOS. */

void listFiles();
void initializeFCB(FCB *fcb, const char *filename);
void createFile(const char *filename);
void deleteFile(const char *filename);

/* temporary initializations, convert into more
detailed FCB defined in FCB.h */

void initializeFCB(FCB *fcb, const char *filename) {
    strncpy(fcb->fileName, filename, sizeof(fcb->fileName)-1);
    fcb->fileName[sizeof(fcb->fileName)-1] = '\0';
    fcb->fileSize = 0; // 0 bytes
}


/*
void listFiles() {
    DIR *d;
    struct dirent *dir;
    struct stat file_stat;
    FCB *fcb;

    d = opendir(".");
    if(d) {
        while((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_REG) {
                initializeFCB(fcb, dir->d_name);
                stat(dir->d_name, &file_stat);
                fcb->fileSize = file_stat.st_size;
                printf("Filename: %s, Size: %u bytes\n", fcb->fileName, fcb->fileSize);
            }
        }
        closedir(d);
    } else {
        printf("Could not open current directory.\n");
    }
}
*/


void listFiles() {
    DIR *d;
    struct dirent *dir;

    d = opendir(".");
    if (d) {
        printf("Files in the directory:\n");
            while ((dir = readdir(d)) != NULL) {
                if (dir->d_type == DT_REG) {
                    printf("%s\n", dir->d_name);
                }
            }
            closedir(d);
        } else {
            printf("Could not open current directory.\n");
        }
    }


void createFile(const char *filename) {
    FILE *file = fopen(filename, "w");
    if(file) {
        printf("File '%s' created successfully.\n", filename);
        fclose(file);
        return;
    }
    printf("Error creating file '%s'.\n", filename);
}


void deleteFile(const char *filename) {
    if(!remove(filename)) {
        printf("File '%s' deleted successfully.\n", filename);
        return;
    }
    printf("Error deleting file '%s'.\n", filename);
}


int main() {
    int user_selection;
    char filename[13];
    while(1) {
        printf("File Manager.c \n");
        printf("1. List files\n");
        printf("2. Create new file\n");
        printf("3. Delete file\n");
        printf("4. Terminate program\n");
        scanf("%d", &user_selection);

    switch(user_selection) {
        case 1:
            printf("Files:\n");
            listFiles(); 
            break;
        case 2:
            printf("Enter the filename to create file: ");
            scanf("%s", filename);
            createFile(filename);
            break;
        case 3:
            printf("Enter filename to delete file: ");
            scanf("%s", filename);
            deleteFile(filename);
            break;
        case 4:
            return 0;
        default:
            printf("Enter a valid input. ");
            break;
        }
    }
    return 0;
}
