#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/times.h>
#include <ctype.h>

#include "generate.h"
#include "sort.h"
#include "copy.h"

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
    //double realTime = countTime(startTime,endTime);
    double userTime = countTime(startTmsTime.tms_utime, endTmsTime.tms_utime);
    double sysTime  = countTime(startTmsTime.tms_stime, endTmsTime.tms_stime);
    
    printf("Operacja: %s\n",opName);
    //printf("Czas rzeczywisty: %f\n",realTime);
    printf("Czas uzytkownika: %f\n",userTime);
    printf("Czas systemowy: %f\n\n\n",sysTime);

    fprintf(output, "\nOperacja: %s\n",opName);
    //fprintf(output, "Czas rzeczywisty: %f\n",realTime);
    fprintf(output, "Czas uzytkownika: %f\n",userTime);
    fprintf(output, "Czas systemowy: %f\n\n\n",sysTime);

}




int main(int argc, char** argv){
    // Inicjalizacja generatora liczb pseudolosowych
    time_t t;
    srand((unsigned) time(&t));

    if (argc < 2){                                                  
        printf("Nie podano argumentow!\n");                         
        exit(1);
    }

    startTimer();
    char * operation = argv[1];
    if(strcmp(operation, "generate") == 0){
        if (argc != 5){
            fprintf(stderr,"Niepoprawna ilosc parametrow dla operacji 'generate'!\n");
            fprintf(stderr,"Sygnatura: generate filename recordCount recordLength\n");
            exit(1);
        }

        const char* filename = argv[2];

        if ( !isANumber(argv[3]) || atoi(argv[3]) <= 0){      // Sprawdzanie, czy recordCount jest dodatnia liczba
            fprintf(stderr,"recordCount (ilosc rekordow) musi byc dodatnia liczba!\n");
            exit(1);
        }
        int recordCount = atoi(argv[3]);

        if ( !isANumber(argv[4]) || atoi(argv[4]) <= 0){      // Sprawdzanie, czy recordLength jest dodatnia liczba
            fprintf(stderr,"recordLength (dlugosc rekordu) musi byc dodatnia liczba!\n");
            exit(1);
        }
        int recordLength = atoi(argv[4]);

        generate(filename, recordCount, recordLength);
        endTimer();
        return 0;
    }
    else if(strcmp(operation, "sort") == 0){
        if (argc != 6){
            fprintf(stderr,"Niepoprawna ilosc parametrow dla operacji 'sort'!\n");
            fprintf(stderr,"Sygnatura: sort filename recordCount recordLength sys/lib\n");
            exit(1);
        }

        const char* filename = argv[2];

        if ( !isANumber(argv[3]) || atoi(argv[3]) <= 0){      // Sprawdzanie, czy recordCount jest dodatnia liczba
            fprintf(stderr,"recordCount (ilosc rekordow) musi byc dodatnia liczba!\n");
            exit(1);
        }
        int recordCount = atoi(argv[3]);

        if ( !isANumber(argv[4]) || atoi(argv[4]) <= 0){      // Sprawdzanie, czy recordLength jest dodatnia liczba
            fprintf(stderr,"recordLength (dlugosc rekordu) musi byc dodatnia liczba!\n");
            exit(1);
        }
        int recordLength = atoi(argv[4]);

        char* mode = argv[5];
        int isLib;
        if ( strcmp(mode,"sys") == 0 )      isLib = 0;
        else if ( strcmp(mode,"lib") == 0 )      isLib = 1;
        else{
            fprintf(stderr, "Nieznany tryb wywolania operacji! Dopuszczalne tryby: sys, lib\n");
            exit(1);
        }

        sort(filename, recordCount, recordLength, isLib);
    }
    else if(strcmp(operation, "copy") == 0){
        if (argc != 7){
            fprintf(stderr,"Niepoprawna ilosc parametrow dla operacji 'copy'!\n");
            fprintf(stderr,"Sygnatura: copy source target recordCount bufferSize sys/lib\n");
            exit(1);
        }

        const char* sourcename = argv[2];

        const char* targetname = argv[3];

        if ( !isANumber(argv[4]) || atoi(argv[4]) <= 0){      // Sprawdzanie, czy recordCount jest dodatnia liczba
            fprintf(stderr,"recordCount (ilosc rekordow) musi byc dodatnia liczba!\n");
            exit(1);
        }
        int recordCount = atoi(argv[4]);

        if ( !isANumber(argv[5]) || atoi(argv[5]) <= 0){      // Sprawdzanie, czy bufferSize jest dodatnia liczba
            fprintf(stderr,"bufferSize (dlugosc bufora) musi byc dodatnia liczba!\n");
            exit(1);
        }
        int bufferSize = atoi(argv[5]);

        char* mode = argv[6];
        int isLib;
        if ( strcmp(mode,"sys") == 0 )      isLib = 0;
        else if ( strcmp(mode,"lib") == 0 )      isLib = 1;
        else{
            fprintf(stderr, "Nieznany tryb wywolania operacji! Dopuszczalne tryby: sys, lib\n");
            exit(1);
        }

        copy(sourcename, targetname, recordCount, bufferSize, isLib);
    }
    else{
        fprintf(stderr,"Nieznana operacja! Dostepne operacje:\n");
        fprintf(stderr,"- generate filename recordCount recordLength\n");
        fprintf(stderr,"- sort filename recordCount recordLength sys/lib\n");
        fprintf(stderr,"- copy source target recordCount bufferSize sys/lib\n");
        exit(1);
    }
    endTimer();
    FILE* resHandle = fopen("wyniki.txt","a");
    writeResults(resHandle, operation);
    fclose(resHandle);
        
    return 0;
}