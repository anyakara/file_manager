#ifndef __FILE_CONTROL_BLOCK_HPP_
#define __FILE_CONTROL_BLOCK_HPP_

#include<sys/_types/_time_t.h>
#include<stdio.h>
#include<string.h>
#include<time.h>

typedef struct {
    char fileName[100];
    unsigned int fileSize;
    char fileType[10];
    unsigned int accessPermissions; // bitmask for permissions

    time_t creationTime;
    time_t lastModifiedTime;
    time_t lastAccessedTime;
    char owner[100];

    unsigned int location;
    unsigned int linkCount;
} FCB;


void initalizeFCB( FCB *fileControlBlock, const char *fileName, unsigned int fileSize,
    const char *fileType, unsigned int accessPermissions, const char *owner, unsigned int location ) {
strcpy(fileControlBlock->fileName, fileName);
fileControlBlock->fileSize = fileSize;
strcpy(fileControlBlock->fileType, fileType);
fileControlBlock->accessPermissions = accessPermissions;
fileControlBlock->creationTime = time(NULL);
fileControlBlock->lastModifiedTime = fileControlBlock->creationTime;
fileControlBlock->lastAccessedTime = fileControlBlock->creationTime;
strcpy(fileControlBlock->owner, owner);
fileControlBlock->location = location;
fileControlBlock->linkCount = 1;
}


void displayFCB(const FCB *fcb) {
    printf("File Name: %s\n", fcb->fileName);
    printf("File Size: %u bytes\n", fcb->fileSize);
    printf("File Type: %s\n", fcb->fileType);
    printf("Access Permissions: %u\n", fcb->accessPermissions);
    printf("Creation Time: %s", ctime(&fcb->creationTime));
    printf("Last Modified Time: %s", ctime(&fcb->lastModifiedTime));
    printf("Last Accessed Time: %s", ctime(&fcb->lastAccessedTime));
    printf("Owner: %s\n", fcb->owner);
    printf("Location: %u\n", fcb->location);
    printf("Link Count: %u\n", fcb->linkCount);
}

#endif