#include "FCB.h"

int main() {
    FCB myFile;
    initalizeFCB(&myFile, 
    "example.txt", 
    1024, ".txt", 
    0b110110100, 
    "user1", 42);
    displayFCB(&myFile);
}   