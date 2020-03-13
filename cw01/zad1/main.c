#include <stdlib.h>
#include <stdio.h>
#include "myLib.h"




// MALY POLIGON TESTOWY
int main(){
    char* filename1 = "a.txt";
    char* filename2 = "a.txt";

    struct BlockArray* blockArray = createBlockArray(1);
    printBlock(blockArray, 0);
    return 0;
}