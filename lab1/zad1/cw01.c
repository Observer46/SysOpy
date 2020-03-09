#include <stdlib.h>
#include <stdio.h>
#include "myLib.h"




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
    /*char* tmpname = "tmp.txt";
    char* filename1 = "a.txt";
    char* filename2 = "b.txt";
    make_tmp_file(filename1, filename2);

    test_read(tmpname);
    int del_status = remove(tmpname);
    if(del_status != 0){
        printf("Nie udalo sie usunac pliku tymczasowego");
        exit(1);
    }
    */
    createBlockArray(10);
    return 0;
}