#include "myLib.h"

struct BlockArray createBlockArray(int size){
    struct BlockArray array;
    array.blockArray = (struct OpBlock*) calloc(size,sizeof(struct OpBlock));
    array.idx = -1;
    return array;
};

struct OpBlock createOpBlock(int size){
    struct OpBlock opBlock;
    opBlock.opArray = (struct char*) calloc(size,sizeof(char*));
    opBlock.opCount = size;
    return opBlock;
};

struct OpBlock compareTwoFiles(const char* file1,const char* file2){
    int opCount = 0;
    
    struct OpBlock opBlock;
    


};
/*
void make_tmp_file(const char* filename1, const char* filename2){
    char cmdbuffor [100];
    sprintf(cmdbuffor,"diff %s %s >>tmp.txt",filename1, filename2);
    system(cmdbuffor);
}*/