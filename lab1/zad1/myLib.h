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
    int opCount;
    struct OpBlock* opArray;
};



struct BlockArray createBlockArray(int size);
struct OpBlock createOpBlock(int size);
struct OpBlock compareTwoFiles(const char* file1,const char* file2);
//struct OpBlock createOpBlock(int size);


#endif