#include <stdlib.h>
#include <stdio.h>
#include "myLib.h"




// MALY POLIGON TESTOWY
int main(){
    char* filename1 = "a.txt";
    char* filename2 = "a.txt";

    struct BlockArray* blockArray = createBlockArray(1);
    char* files[] = {filename1, filename2};
    parseFilePairs(blockArray, files, 1);
    
    int operation = blockArray->blocks[0]->opCount;
    printf("%d\n",operation);
    printOperation(blockArray,10,2,1);
    printBlock(blockArray, 0);
    deleteBlockArray(blockArray);
    return 0;
}