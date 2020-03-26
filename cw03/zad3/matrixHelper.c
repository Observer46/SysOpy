#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "matrixMethods.h"

// UWAGA: Przyjeta konwecja - na kazda liczbe w pliku przeznaczone jest 10 znakow



void generate_matrix(FILE* target, int rows, int cols){     // Do pliku target macierz o rozmiarach rows x cols, elementy [-100,                                                                                                                                100]

    char* row = (char*)calloc(cols*size_of_element() + 2, sizeof(char));   //size_of_element() - rozmiar pojedynczego elementu w                                                                              macierzy, przeznaczone na to 10 znakow; +2 dla                                                                                  bezpieczenstwa 
    for(int i=0; i < rows; i++){
        strcpy(row,"");
        
        for(int j=0; j < cols; j++){                // Budujemy wiersz macierzy
            int number = rand() % 201 - 100;        // [-100, 100] - elementy macierzy
            char numberAsString[size_of_element()];
            sprintf(numberAsString,"%d ",number);
            for (int k=strlen(numberAsString); k < size_of_element(); k++)  // Reszte miejsca na liczbe wypelniamy spacjami
                strcat(numberAsString," ");
            strcat(row,numberAsString);
        }
        strcat(row, "\n");
        fwrite(row, sizeof(char), strlen(row), target);
    }
    
    free(row);
}

void generate(int pairCount, int minDim, int maxDim){
    int dimInterval = maxDim - minDim + 1;  // Szerokosc przedzialu dla wymierow macierzy
    FILE* listFile = fopen("listAll.txt", "w");   // Plik z lista par macierzy
    for (int i=0; i < pairCount; i++){

        int rows1 = rand() % dimInterval + minDim;
        int cols1rows2 = rand() % dimInterval + minDim;
        int cols2 = rand() % dimInterval + minDim;

        char firstMatrixName[21]; 
        char secondMatrixName[21];
        char outputMatrixName[22];
        sprintf(firstMatrixName,"matrix%dA.txt",i);
        sprintf(secondMatrixName,"matrix%dB.txt",i);
        sprintf(outputMatrixName,"matrix%dC.txt",i);

        FILE* firstMatrix = fopen(firstMatrixName, "w");    // Nadpisuje poprzedni
        generate_matrix(firstMatrix, rows1, cols1rows2);
        fclose(firstMatrix);

        FILE* secondMatrix = fopen(secondMatrixName, "w");  // Nadpisuje poprzedni
        generate_matrix(secondMatrix, cols1rows2, cols2);
        fclose(secondMatrix);

        sprintf(firstMatrixName,"%s ",firstMatrixName);
        sprintf(secondMatrixName,"%s ",secondMatrixName);
        sprintf(outputMatrixName,"%s\n",outputMatrixName);

        fwrite(firstMatrixName, sizeof(char), strlen(firstMatrixName), listFile);
        fwrite(secondMatrixName, sizeof(char), strlen(secondMatrixName), listFile);
        fwrite(outputMatrixName, sizeof(char), strlen(secondMatrixName), listFile);

    }
    fclose(listFile);
}
int check(char* inputMat1, char* inputMat2, char* mulResMat){
    FILE* matA = fopen(inputMat1,"r");
    FILE* matB = fopen(inputMat2, "r");
    FILE* matC = fopen(mulResMat, "r");
    if (matA == NULL){
        fprintf(stderr, "Nie udalo sie otwrzyc pliku %s!\n", inputMat1);
        exit(1);
    }
    if (matB == NULL){
        fprintf(stderr, "Nie udalo sie otwrzyc pliku %s!\n", inputMat2);
        exit(1);
    }
    if (matC == NULL){
        fprintf(stderr, "Nie udalo sie otwrzyc pliku %s!\n", mulResMat);
        exit(1);
    }

    int resRows = count_rows(matA);
    int resCols = count_cols(matB);
    int elemMulCount = count_cols(matA);

    for (int i=0; i < resRows; i++)
        for (int j=0; j < resCols; j++){
            int expectedMulRes = 0;
            for (int k=0; k < elemMulCount; k++)
                expectedMulRes += matrix_elem(matA, i, k) * matrix_elem(matB, k, j);
            if ( expectedMulRes != matrix_elem(matC, i, j) ){
                fprintf(stderr,"[%d][%d] | Oczekiwano: %d, uzyskano: %d\n",i,j,expectedMulRes,matrix_elem(matC, i, j));
                return 0;
            }
        }

    fclose(matA);
    fclose(matB);
    fclose(matC);

    return 1;
}


int main(int argc, char** argv){
    time_t t;
    srand((unsigned) time(&t));
    // create_empty_output("matrix0C.txt", 3, 5);
    // FILE* mat = fopen("matrix0C.txt","r+");
    // FILE* matA = fopen("matrix0A.txt","r");
    // FILE* matB = fopen("matrix0B.txt","r");
    // int mCols = count_cols(matB);
    // int mRows = count_rows(matA);
    // int mulTimes = count_cols(matA);
    // for(int i=0; i < mRows; i++){
    //     for(int j=0; j < mCols; j++){
    //         int number = 0;
    //         for(int k=0; k < mulTimes; k++)
    //             number += matrix_elem(matA, i, k) * matrix_elem(matB, k, j);
    //         write_to_matrix(mat, number, i, j);
    //     }
    // }
    // //printf("%d\n",matrix_elem(mat, 2,2));
    
    // if ( check("matrix0A.txt", "matrix0B.txt", "matrix0C.txt"))
    //     printf("OKE!\n");
    // else
    //     printf("CUS NIE TAK!\n");
    // fclose(matA);
    // fclose(matB);
    // fclose(mat);
    if (argc < 2){
        fprintf(stderr,"Nie podano komendy do wywolania dla matrixHelper!\n");
        printf("Mozliwe komendy:\n");
        printf("generate n min max\n");
        printf("check A B C\n");
        exit(1);
    }
    char* cmd = argv[1];
    if  ( strcmp(cmd,"generate") == 0 ){
        if(argc != 5){
            fprintf(stderr,"Niepoprawna liczba argumentow! Sygnatura komendy: generate n min max\n");
            exit(1);
        }
        
        if ( !isANumber(argv[2]) ){
            fprintf(stderr,"n - ilosc tworzonych par macierzy musi byc dodatnia liczba!\n");
            exit(1);
        }
        if ( !isANumber(argv[3]) ){
            fprintf(stderr,"min - minimalny wymiar macierzy musi byc dodatnia liczba!\n");
            exit(1);
        }
        if ( !isANumber(argv[4]) ){
            fprintf(stderr,"max - maksymalny wymiar macierzy musi byc dodatnia liczba!\n");
            exit(1);
        }
        int pairCount = atoi(argv[2]);
        int minDim = atoi(argv[3]);
        int maxDim = atoi(argv[4]);
        if (minDim > maxDim){
            int tmp = maxDim;
            maxDim = minDim;
            minDim = tmp;
        }
        generate(pairCount, minDim, maxDim);
        printf("Poprawnie wygenerowano %d par(y) macierzy o wymiarach z przedzialu %d-%d.\n", pairCount, minDim, maxDim);
    }
    else if ( strcmp(cmd,"check") == 0){
        if(argc != 5){
            fprintf(stderr,"Niepoprawna liczba argumentow! Sygnatura komendy: check A B C\n");
            exit(1);
        }
        char* inputMat1 = argv[2];
        char* inputMat2 = argv[3];
        char* mulResMat = argv[4];

        if ( check(inputMat1, inputMat2, mulResMat) )
            printf("%s * %s -> %s: poprawny wynik.\n",inputMat1, inputMat2, mulResMat);
        else
            printf("Mnozenie macierzy %s %s w pliku wynikowym %s jest NIEPOPRAWNE!\n",inputMat1, inputMat2, mulResMat);
    }
    else{
        printf("Nieznana komenda! Dostepne komedny:\n");
        printf("generate n min max - generowanie n-par macierzy o losowych wymiarach z przedzialu [min,max] (macierze wypelnione liczbami [-100,100]\n");
        printf("check A B C - sprawdza czy mnozenie macierzy A*B daje macierz C\n");
    }

    return 0;
}