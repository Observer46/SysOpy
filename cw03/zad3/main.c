#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/times.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "matrixMethods.h"

// clock_t startTime, endTime;
// struct tms startTmsTime, endTmsTime;

// Funkcje pomocnicze
////////////////////////////////////////////

void errorExit(){
    exit(1);
}

void deallocAll(char** matricesA, char** matricesB, char** matricesC, int pairCount, pid_t* farm){
    if(farm != NULL)
        free(farm);
    for(int i=0; i<pairCount; i++){
        if(matricesA[i] != NULL)
            free(matricesA[i]);
        if(matricesB[i] != NULL)
            free(matricesB[i]);
        if(matricesC[i] != NULL)
            free(matricesC[i]);
    }
    if(matricesA != NULL)
        free(matricesA);
    if(matricesB != NULL)
        free(matricesB);
    if(matricesC != NULL)
        free(matricesC);
}

void deallocErrExit(char** matricesA, char** matricesB, char** matricesC, int pairCount, pid_t* farm){
    deallocAll(matricesA, matricesB, matricesC, pairCount, farm);
    exit(1);
}

// void startTimer(){
//     startTime = times(&startTmsTime);
// }

// void endTimer(){
//     endTime = times(&endTmsTime);
// }

double countTime(clock_t start, clock_t end){
    return ( (double) (end - start)/CLOCKS_PER_SEC);
}

int min(int a, int b){
    return a < b ? a : b;
}

int max(int a, int b){
    return a > b ? a : b;
}

/////////////////////////////////////////

void parse_list(FILE* listFile, int pairCount, char** matricesA, char** matricesB, char** matricesC){
    char nameBuffer[25];

    for(int i=0; i < pairCount; i++){
        fscanf(listFile,"%s",nameBuffer);
        matricesA[i] = (char*)calloc(strlen(nameBuffer), sizeof(char));
        strcpy(matricesA[i],nameBuffer);

        fscanf(listFile,"%s",nameBuffer);
        matricesB[i] = (char*)calloc(strlen(nameBuffer), sizeof(char));
        strcpy(matricesB[i],nameBuffer);

        fscanf(listFile,"%s",nameBuffer);
        matricesC[i] = (char*)calloc(strlen(nameBuffer), sizeof(char));
        strcpy(matricesC[i],nameBuffer);
    }
}

void create_output_files(char** matricesA, char** matricesB, char** matricesC, int pairCount, pid_t* farm){
    for(int i=0; i < pairCount; i++){
        FILE* matA = fopen(matricesA[i],"r");
        FILE* matB = fopen(matricesB[i],"r");

        if(matA == NULL){
            fprintf(stderr,"Nie udalo sie odczytac macierzy z pliku %s!\n", matricesA[i]);
            deallocErrExit(matricesA, matricesB, matricesC, pairCount, farm);
        }
        if(matB == NULL){
            fprintf(stderr,"Nie udalo sie odczytac macierzy z pliku %s!\n", matricesB[i]);
            deallocErrExit(matricesA, matricesB, matricesC, pairCount, farm);
        }

        int outRows = count_rows(matA);
        int outCols = count_cols(matB);

        create_empty_output(matricesC[i], outRows, outCols);

        fclose(matA);
        fclose(matB);
    }
}

// Korzysta z matrixMethods.h
int mul_matrix(int processIdx, int* processCount, int* pairCount, char** matricesA, char** matricesB, char** matricesC, int* isShared, int* timeLimit, pid_t* farm){       
    int mulCounter = 0;

    int isFormated = *isShared ? 1 : 0;
    clock_t startTime;
    startTime = clock();

    for(int i=0; i < *pairCount; i++){
        char* matNameA = matricesA[i];
        char* matNameB = matricesB[i];
        char* matNameC = matricesC[i];

        FILE* matA = fopen(matNameA,"r");
        FILE* matB = fopen(matNameB,"r");

        if(matA == NULL){
            fprintf(stderr,"Nie udalo sie odczytac macierzy z pliku %s!\n", matNameA);
            deallocErrExit(matricesA, matricesB, matricesC, *pairCount, farm);
        }
        if(matB == NULL){
            fprintf(stderr,"Nie udalo sie odczytac macierzy z pliku %s!\n", matNameB);
            deallocErrExit(matricesA, matricesB, matricesC, *pairCount, farm);
        }

        int colsInB = count_cols(matB);
        int colsPerProc = colsInB / *processCount;
        int unmatchedCols = colsInB % *processCount;     // Tyle kolumn jeszcze trzeba rozdac 

        int mulRows = count_rows(matA);
        int mulCols = count_cols(matA);
        int startingCol = (colsPerProc + 1) * min(unmatchedCols, processIdx) + colsPerProc * max(0, processIdx - unmatchedCols);
        if(processIdx < unmatchedCols) colsPerProc++;     // Pierwsze procesy wykonuja mnozenie jednej kolumny wiecej jesli nie dzieli sie rowno


        FILE* output;
        if (*isShared)
            output = fopen(matNameC, "r+");
        else{
            char outputName[30];
            sprintf(outputName,"%d%s", getpid(), matNameC);
            create_empty_output(outputName, mulRows, colsPerProc);
            output = fopen(outputName,"r+");
        }
        if (output == NULL){
            fprintf(stderr,"Nie udalo sie znalezc pliku wyjsciowego podczas mnozenia!\n");
            deallocErrExit(matricesA, matricesB, matricesC, *pairCount, farm);
        }

        

        for(int i = 0; i < colsPerProc; i++){
            int targetCol = startingCol + i;
            mul_col(mulRows, mulCols, targetCol, matA, matB, output, isFormated, i);    // Tutaj moznaby uzyc FLOCK do zablokowania pliku lecz przy mojej reprezentacji macierzy procesy nie
        }                                                                // beda sobie wchodzic w droge, nawet dzialajac na tym samym pliku wiec nie ma potrzeby


        fclose(matA);
        fclose(matB);
        fclose(output);
    
        mulCounter++;
        clock_t endTime;        // Jakos inaczej mierzyc czas
        endTime = clock();
        
        int timeInSecs = (int) countTime(startTime, endTime);    
        if (timeInSecs >= *timeLimit){
            printf("Przerywam %d bo czas sie skonczyl\n",getpid());
            break;
        }
    }
    printf("%d konczy dzialanie, wykonalem %d mnozen\n",getpid(),mulCounter);
    return mulCounter;
}

void paste_files(char** matricesC, pid_t* farm, int processCount, int pairCount){
    const int fileBufSize = 30;
    char** pasteArgs = (char**)calloc(processCount + 3, sizeof(char*));
    pasteArgs[0] = (char*)calloc(10,sizeof(char));
    for(int i=1; i < processCount + 1; i++)
        pasteArgs[i] = (char*)calloc(fileBufSize, sizeof(char));
    pasteArgs[processCount + 1] = (char*)calloc(16,sizeof(char));
    

    for(int i=0; i < pairCount; i++){
        char* matNameC = matricesC[i];
        strcpy(pasteArgs[0],"paste");
        strcpy(pasteArgs[processCount+1],"--delimiters=");

        for(int j=0; j < processCount; j++)      // pasteArgs <-- tablica z plikami, w odpowiedniej kolejnosci, na koncu opcja laczenia 
            sprintf(pasteArgs[j+1],"%d%s", farm[j], matNameC);

        pid_t proc = fork();
        if (proc == 0){
            int fileDesc = open(matNameC, O_WRONLY|O_TRUNC);
            dup2(fileDesc,1);       // Podmieniamy standardowe wyjscie na deskryptor naszego pliku wyjsciowego
            close(fileDesc);    
            execvp("paste", pasteArgs);      // paste -> fileDesc
            fprintf(stderr,"Paste nie powiodlo sie!\n");
            exit(1);
        }
        else
            wait(0);
        
        
        strcpy(pasteArgs[0],"rm");
        strcpy(pasteArgs[processCount + 1],"-f");
        
        proc = fork();
        if (proc == 0){
            execvp("rm",pasteArgs);
            fprintf(stderr,"Usuwanie plikow tymczasowych nie powiodlo sie!\n");
            exit(1);
        }
        else
            wait(0);
    }
    
    for(int i=0; i < processCount + 2; i++)
        free(pasteArgs[i]);
    free(pasteArgs);
}



int main(int argc, char** argv){
    if (argc != 5 ){
        fprintf(stderr,"Niepoprawna liczba argumentow dla programu macierz!\n");
        printf("Potrzebne argumenty: plik_lista liczba_procesow limit_czasu shared/separated\n");
        errorExit();
    }

    if ( !isANumber(argv[2]) ){
        fprintf(stderr,"Liczba procesow do utowrzenia musi byc dodatnia liczba!\n");
        errorExit();
    }
    if ( !isANumber(argv[3]) ){
        fprintf(stderr,"Limit czasu (w sekundach) musi byc dodatnia liczba!\n");
        errorExit();
    }
    if ( strcmp(argv[4],"shared") != 0 && strcmp(argv[4],"separated") != 0 ){
        fprintf(stderr,"Program mozna wywolac tylko w dwoch trybach: shared/separated (wspoldzielone/osobne pliki wynikowe)\n");
        errorExit();
    }

    char* listName = argv[1];
    int processCount = atoi(argv[2]);
    int timeLimit = atoi(argv[3]);
    int isShared = strcmp(argv[4],"shared") == 0 ? 1 : 0;

    FILE* listFile = fopen(listName, "r");
    if (listFile == NULL){
        fprintf(stderr, "Nie udalo sie otworzyc pliku %s bedacego lista par!\n",listName);
        errorExit();
    }
    
    int pairCount = count_rows(listFile);
    char** matricesA = (char**)calloc(pairCount, sizeof(char*)); 
    char** matricesB = (char**)calloc(pairCount, sizeof(char*));
    char** matricesC = (char**)calloc(pairCount, sizeof(char*));
    pid_t* farm = (pid_t*)calloc(processCount, sizeof(pid_t));
    parse_list(listFile, pairCount, matricesA, matricesB, matricesC);  // Od tego momentu wyscie z bledem to deallocErrExit(...)
    fclose(listFile);

    create_output_files(matricesA, matricesB, matricesC, pairCount, farm);

    for (int i=0; i < processCount; i++){   // Farma rusza 
        pid_t childPid = fork();

        if (childPid == 0){     // Potomek mnozy
            
            int mulCounter = 0;
            mulCounter = mul_matrix(i, &processCount, &pairCount, matricesA, matricesB, matricesC, &isShared, &timeLimit, farm);
            exit(mulCounter);       // Tutaj umiera proces
        }
        else{              
            farm[i] = childPid;
        }
    }

    waitpid(0, NULL, 0);        // Czekamy na wszystkie dzieciaczki (Przodek czeka)
    

    for(int i=0; i < processCount; i++){
        int exitStatus;
        waitpid(farm[i], &exitStatus, 0);
        printf("Proces: %d wykonal %d mnozen.\n", farm[i], WEXITSTATUS(exitStatus));
    }
    if (!isShared)
        paste_files(matricesC, farm, processCount, pairCount);

    deallocAll(matricesA, matricesB, matricesC, pairCount, farm);
    return 0;
}