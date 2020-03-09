#include "myLib.h"

struct BlockArray* createBlockArray(int size){
    struct BlockArray* array = (struct BlockArray*) calloc(1,sizeof(struct BlockArray));
    array -> blocks = (struct OpBlock*) calloc(size,sizeof(struct OpBlock));
    array -> idx = -1;  // Do zastanowienia
    return array;
};

struct OpBlock* createOpBlock(int size){
    struct OpBlock* opBlock = (struct OpBlock*)calloc(1,sizeof(struct OpBlock));
    opBlock -> operations = (char**) calloc(size,sizeof(char*));
    opBlock -> idx = size;
    return opBlock;
};

struct OpBlock* compareTwoFiles(const char* file1,const char* file2){
    int opCount = 0;

    char cmdbuffor [100];       //na razie troche brzydko
    sprintf(cmdbuffor,"diff %s %s >tmp.txt",file1, file2);
    system(cmdbuffor);
    FILE* tmpFile = fopen("tmp.txt","r");
    if(tmpFile == NULL){
        printf("Nie udalo sie otworzyc pliku tymczasowego - blad programu!");
        exit(1);
    }

    
    struct OpBlock* opBlock = createOpBlock(opCount);

    return opBlock;


};
