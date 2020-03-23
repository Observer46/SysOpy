#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>


int isANumber(char* number){
    for(int i=0; i < strlen(number); i++)
        if ( !isdigit(number[i]) )
            return 0;
    return 1;
}


void generate_matrix(FILE* target, int rows, int cols){     // Do pliku target macierz o rozmiarach rows x cols, elementy [-100,100]
    char* row = (char*)calloc(cols*5 + 2, sizeof(char));   //5* - liczba ma do 4 symboli (znak + 3 cyfry) i jedno na znak bialy, +2 dla bezpieczenstwa na skrajne przypadki
    for(int i=0; i < rows; i++){
        strcpy(row,"");
        
        for(int j=0; j < cols; j++){                // Budujemy wiersz macierzy
            int number = rand() % 201 - 100;        // [-100, 100] - elementy macierzy
            char numberAsString[5];
            sprintf(numberAsString,"%d ",number);
            strcat(row,numberAsString);
        }
        strcat(row, "\n");
        fwrite(row, sizeof(char), strlen(row), target);
    }
    
    free(row);
}

void generate(int pairCount, int minDim, int maxDim){
    int dimInterval = maxDim - minDim + 1;  // Szerokosc przedzialu dla wymierow macierzy
    for (int i=0; i < pairCount; i++){

        int rows1 = rand() % dimInterval + minDim;
        int cols1rows2 = rand() % dimInterval + minDim;
        int cols2 = rand() % dimInterval + minDim;

        char firstMatrixName[20]; 
        char secondMatrixName[20];
        sprintf(firstMatrixName,"matrix%dA.txt",i);
        sprintf(secondMatrixName,"matrix%dB.txt",i);

        FILE* firstMatrix = fopen(firstMatrixName, "w");    // Nadpisuje poprzedni
        generate_matrix(firstMatrix, rows1, cols1rows2);
        fclose(firstMatrix);

        FILE* secondMatrix = fopen(secondMatrixName, "w");  // Nadpisuje poprzedni
        generate_matrix(secondMatrix, cols1rows2, cols2);
        fclose(secondMatrix);

    }
}
void check(char* inputMat1, char* inputMat2, char* mulResMat){
    // TODO
}


int main(int argc, char** argv){
    time_t t;
    srand((unsigned) time(&t));
    if (argc < 2){
        fprintf(stderr,"Nie podano komendy do wywolania dla matrixHelper!\n");
        exit(1);
    }
    char* cmd = argv[1];
    if  ( strcmp(cmd,"generate") == 0 ){
        if(argc != 5){
            fprintf(stderr,"Niepoprawna liczba argumentow! Sygnatura komendy: generate n min max\n");
            exit(1);
        }
        
        if ( !isANumber(argv[2]) || atoi(argv[2]) <= 0){
            fprintf(stderr,"n - ilosc tworzonych par macierzy musi byc dodatnia liczba!\n");
            exit(1);
        }
        if ( !isANumber(argv[3]) || atoi(argv[3]) <= 0){
            fprintf(stderr,"min - minimalny wymiar macierzy musi byc dodatnia liczba!\n");
            exit(1);
        }
        if ( !isANumber(argv[4]) || atoi(argv[4]) <= 0){
            fprintf(stderr,"n - maksymalny wymiar macierzy musi byc dodatnia liczba!\n");
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
    }
    else if ( strcmp(cmd,"check") == 0){
        if(argc != 5){
            fprintf(stderr,"Niepoprawna liczba argumentow! Sygnatura komendy: check A B C\n");
            exit(1);
        }
        char* inputMat1 = argv[2];
        char* inputMat2 = argv[3];
        char* mulResMat = argv[4];

        check(inputMat1, inputMat2, mulResMat);
    }
    else{
        printf("Nieznana komenda! Dostepne komedny:\n");
        printf("generate n min max - generowanie n-par macierzy o losowych wymiarach z przedzialu [min,max] (macierze wypelnione liczbami [-100,100]\n");
        printf("check A B C - sprawdza czy mnozenie macierzy A*B daje macierz C\n");
    }

    return 0;
}