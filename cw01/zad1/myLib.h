#ifndef _myLib_h
#define _myLib_h


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct OpBlock{
    char ** operations;
    int opCount;
};

struct BlockArray{
    int size;
    int iter;
    struct OpBlock** blocks;
};



struct BlockArray* createBlockArray(int size);
struct OpBlock* createOpBlock(int size);
int parseOperationsFromTmp(struct OpBlock* opBlock, char* tmpfileName);
int compareTwoFiles(struct BlockArray* blockArray, const char* file1,const char* file2, int idx);
void parseFilePairs(struct BlockArray* blockArray ,char** filePairsList, int pairsNumber);

//wyniki traktujemy jak kody bledow
int deleteOperation(struct BlockArray* blockArray, int blockIdx, int operationIdx);
int deleteOpBlock(struct BlockArray* blockArray, int blockIdx, int printMode);
int deleteBlockArray(struct BlockArray* blockArray);

//bonus do weryfikacji wynikow
void printOperation(struct BlockArray* blockArray, int blockIdx, int operationIdx, int printMode);
void printBlock(struct BlockArray* blockArray, int blockIdx);




#endif