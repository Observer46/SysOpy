#include <stdlib.h>
#include <stdio.h>
#include "myLib.h"


void make_tmp_file(const char* filename1, const char* filename2){
    char cmdbuffor [100];
    sprintf(cmdbuffor,"diff %s %s >tmp.txt",filename1, filename2);
    system(cmdbuffor);
}


void test_read(const char* tmpfile){
    FILE *fileptr = fopen(tmpfile, "r");
    if(fileptr == NULL){
        printf("Nie udalo sie otworzyc pliku tymczasowego :(");
        exit(1);
    }
    fclose(fileptr);
}

void make_tmp(/*PARY PLIKÓW*/){
    //FOR KAŻDA PARA PLIKÓW append_tmp_file
}



// POLIGON TESTOWY
int main(){
    //char* tmpname = "tmp.txt";
    char* filename1 = "a.txt";
    char* filename2 = "b.txt";
    /*
    make_tmp_file(filename1, filename2);

    test_read(tmpname);
    int del_status = remove(tmpname);
    if(del_status != 0){
        printf("Nie udalo sie usunac pliku tymczasowego");
        exit(1);
    }
    
    createBlockArray(10);
    */
    struct BlockArray* blockArray = createBlockArray(1);
    char* files[] = {filename1, filename2};
    parseFilePairs(blockArray, files, 1);
    int operation = blockArray->blocks[0]->opCount;
    printf("%d\n",operation);
    //char* op1 = blockArray ->blocks[0]->operations[1];
    //printf("%s",op1);
    printOperation(blockArray,0,2);
    printBlock(blockArray, 0);
    deleteBlockArray(blockArray);
    return 0;
}