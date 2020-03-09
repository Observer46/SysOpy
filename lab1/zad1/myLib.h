#ifndef _myLib_h
#define _myLib_h


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct OpBlock{
    char ** operations;
    int idx;
};

struct BlockArray{
    int idx;
    struct OpBlock* blocks;
};


// To jeszcze trzeba zweryfikowac
struct BlockArray* createBlockArray(int size);
struct OpBlock* createOpBlock(int size);
struct OpBlock* compareTwoFiles(const char* file1,const char* file2);

// Tego jeszcze nie ma
/*
void parseFilePairs(char* filePairsList);
void deleteOperation(struct BlockArray* blockArray, int blockIdx, int operationIdx);
void deleteOpBlock(struct BlockArray* blockArray, int blockIdx);
void deleteBlockArray(struct BlockArray* blockArray);
*/



#endif