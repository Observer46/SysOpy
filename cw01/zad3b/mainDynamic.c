#include <stdlib.h>
#include <stdio.h>
#include "myLib.h"
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include <dlfcn.h>

clock_t startTime, endTime;
struct tms startTmsTime, endTmsTime;

void startTimer(){
    startTime = times(&startTmsTime);
}

void endTimer(){
    endTime = times(&endTmsTime);
}


int isANumber(char* number){
    for(int i=0; i < strlen(number); i++)
        if ( !isdigit(number[i]) )
            return 0;
    return 1;
}

double countTime(clock_t start, clock_t end){
    return ( (double) (end - start)/(sysconf(_SC_CLK_TCK)) );     // 100 bo tyle razy powtarzamy kazda operacje
}

void writeResults(FILE* output, char* opName){
    endTimer();
    double realTime = countTime(startTime,endTime);
    double userTime = countTime(startTmsTime.tms_utime, endTmsTime.tms_utime);
    double sysTime  = countTime(startTmsTime.tms_stime, endTmsTime.tms_stime);
    
    printf("\nOperacja: %s\n",opName);
    printf("Czas rzeczywisty: %f\n",realTime);
    printf("Czas uzytkownika: %f\n",userTime);
    printf("Czas systemowy: %f\n",sysTime);

    fprintf(output, "\nOperacja: %s\n",opName);
    fprintf(output, "Czas rzeczywisty: %f\n",realTime);
    fprintf(output, "Czas uzytkownika: %f\n",userTime);
    fprintf(output, "Czas systemowy: %f\n\n\n",sysTime);

}



// POLIGON TESTOWY
int main(int argc, char** argv){
    void *libHandle = dlopen("./libmyLib.so",RTLD_LAZY);
    if(libHandle == NULL){
        printf("Nie znaleziono biblioteki!");
        exit(1);
    }

    struct BlockArray* (*createBlockArray)() = dlsym(libHandle, "createBlockArray");
    void (*parseFilePairs)() = dlsym(libHandle, "parseFilePairs");
    int (*deleteOperation)() = dlsym(libHandle, "deleteOperation");
    int (*deleteOpBlock)() = dlsym(libHandle, "deleteOpBlock");
    int (*deleteBlockArray)() = dlsym(libHandle, "deleteBlockArray");



    FILE* output = fopen("results3b.txt","a");
    char currentOperation[25] = "";
      
    startTime = times(&startTmsTime);                                                         
    if (argc < 2){                                                  
        printf("Nie podano argumentow!\n");                         
        exit(1);
    }

    int errorExitFlag = 0;
    struct BlockArray* blockArray = NULL;


    for(int i=1; i < argc; i++){     // i=1, bo pomijamy nazwe programu
        startTimer();
        char* command = argv[i];
        
        if ( strcmp(command,"create_table") == 0 ){     // if command == "create_table"
            strcpy(currentOperation, "create_table");
            
            if (blockArray != NULL){
                printf("Juz stworzono tablice blokow!\n");
                errorExitFlag = 6;
                break;
            }

            if ( i+1 >= argc || !isANumber(argv[i+1]) ){
                printf("Nie podano rozmiaru tablicy blokow!\n");
                errorExitFlag = 1;
                break;
            }

            i++;        // argv[i] = rozmiar tablicy
            int arraySize = atoi(argv[i]);        // rzutowanie na inta rozmiaru
            blockArray = createBlockArray(arraySize);
            printf("Pomyslnie stworzono tablice blokow o rozmiarze %d.\n",arraySize);
        }
        else if ( strcmp(command,"compare_pairs") == 0 ){     // if command == "compare_pairs"
            
            strcpy(currentOperation, "compare_pairs");

            if ( i+1 >= argc ){
                printf("Nie podano sekwencji plikow!\n");
                errorExitFlag = 2;
                break;
            }
            i++;        //argv[i] = pierwsza para plikow (potencjalnie)
            
            int filePairsNumber = 0;

            for(int j = i; j < argc; j++){          // Liczenie plikow w sekwencji
                char* filePair = argv[j];
                //printf("\n%s\n",filePair);
                char* seqCheck = strchr(filePair,':');
                //printf("%s\n",seqCheck);
                if(seqCheck == NULL)    break;
                filePairsNumber++;
            }
            

            
            
            char** filesTab = (char**)calloc(2*filePairsNumber, sizeof(char*));     // Tworzenie tablicy plikow na podstawie ktorej uzupelnimy blockArray
            for(int j=0; j < filePairsNumber; j++){
                char* file1 = (char*)calloc(strlen(argv[i]), sizeof(char));
                strcpy(file1, argv[i]);
                char* file2 = strtok_r(file1,":",&file1);              //UWAGA
                filesTab[2*j] = file1;
                filesTab[2*j + 1] = file2;
                i++;
            }
            i--;
            

            parseFilePairs(blockArray, filesTab, filePairsNumber);
            
            printf("Pomyslnie wczytano sekwencje plikow.\n");
        }
        else if ( strcmp(command,"remove_block") == 0 ){
            strcpy(currentOperation, "remove_block");

            if ( i+1 >= argc || !isANumber(argv[i+1]) ){
                printf("Nie podano numeru bloku do usuniecia!\n");
                errorExitFlag = 3;
                break;
            }

            i++;      // argv[i] = numer bloku
            int blockIdx = atoi(argv[i]);
            int printErrMS = 1;                     // Wywolujemy tak, by wypisalo wiadomosci o bledach
            deleteOpBlock(blockArray, blockIdx, printErrMS);
            printf("Pomyslnie usunieto blok %d.\n", blockIdx);
        }
        else if ( strcmp(command,"remove_operation") == 0 ){
            strcpy(currentOperation, "remove_operation");

            if ( i+1 >= argc || !isANumber(argv[i+1]) ){
                printf("Nie podano numeru bloku z ktorego usuwamy!\n");
                errorExitFlag = 4;
                break;
            }
            if ( i+2 >= argc || !isANumber(argv[i+2]) ){
                printf("Nie podano numeru operacji do usuniecia!\n");
                errorExitFlag = 4;
                break;
            }
            i++;      // argv[i] = numer bloku
            int blockIdx = atoi(argv[i]);

            i++;      // argv[i] = numer operacji
            int operationIdx = atoi(argv[i]);
            deleteOperation(blockArray, blockIdx, operationIdx);
            printf("Pomyslnie usunieto z bloku %d operacje %d.\n", blockIdx, operationIdx);
        }
        else {
            printf("\n%s\n",argv[i]);
            printf("Podano niepoprawna operacje do wykonania!\n");
            errorExitFlag = 5;
            break;
        }
            
        writeResults(output, currentOperation);

    }
   
    deleteBlockArray(blockArray);
 
    // Handler
    printf("\n\n");
    if (errorExitFlag == 1)     printf("Tworzenie tablicy: create_table size\n");
    if (errorExitFlag == 2)     printf("Sekwencja plikow: compare_pairs file1A.txt:file1B.txt file2A.txt:file2B.txt ...\n");
    if (errorExitFlag == 3)     printf("Usuwanie bloku: remove_block index\n");
    if (errorExitFlag == 4)     printf("Usuwanie operacji z bloku: remove_operation block_index operation_index\n");
    if (errorExitFlag == 5)     printf("Dostepne operacje:\n-create_table size\n-compare_pairs file1A.txt:file1B.txt file2A.txt:file2B.txt ...\n-remove_block index\n-remove_operation block_index operation_index\n");

    printf("Koniec dzialania - tablicy blokow juz nie ma w pamieci\n\n\n");
    
    dlclose(libHandle);
    if (errorExitFlag)          exit(1);



    

    return 0;
}