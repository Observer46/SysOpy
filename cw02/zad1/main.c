#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/times.h>
#include <ctype.h>
clock_t startTime, endTime;
struct tms startTmsTime, endTmsTime;



char* create_record(int length){
    char* record = (char*) calloc(length + 1,sizeof(char));     // +1 dla \n
    for(int i=0; i < length; i++)
        record[i] = rand() % 95 + 32;       // Pozwala, by znaki byly czytelne
    record[length] = '\n';
    return record;
}

void generate(const char* filename, int recordCount, int recordLength){
    // Checking if file already exists
    
    FILE* output = fopen(filename,"r");

    if (output != NULL){
        char answer;
        printf("Juz istnieje plik o takiej nazwie. Czy chcesz go nadpisac? (y/n)\n");
        do{
            scanf("%c",&answer);
        }while(answer != 'n' && answer != 'y');

        if(answer == 'n'){
            printf("Nie nadpisuje. Plik nie zostal wygenerowany\n");
            return;
        }
        fclose(output);
        printf("Poprawnie wygenerowano plik %s zawierajacy %d rekordow dlugosci %d.\n", filename, recordCount, recordLength);
    }
    
    
    output = fopen(filename,"w");
    for(int i=0; i < recordCount; i++){
        char* record = create_record(recordLength);
        fwrite(record, sizeof(char), recordLength + 1, output);
        free(record);
    }
    fclose(output);
}




void swap_sys(int fileToSort, int idx1, int idx2, int recordLength){
    char* record1 = (char*) calloc(recordLength + 1, sizeof(char));
    char* record2 = (char*) calloc(recordLength + 1, sizeof(char));

    // Wyciagamy recordy do zamiany
    lseek(fileToSort, (recordLength + 1)*idx1, SEEK_SET);
    read(fileToSort, record1, recordLength + 1);
    lseek(fileToSort, (recordLength + 1)*idx2, SEEK_SET);
    read(fileToSort, record2, recordLength + 1);

    // Zapisujemy je na zamienionych pozycjach
    lseek(fileToSort, (recordLength + 1)*idx1, SEEK_SET);
    write(fileToSort, record2, recordLength + 1);
    lseek(fileToSort, (recordLength + 1)*idx2, SEEK_SET);
    write(fileToSort, record1, recordLength + 1);

    free(record1);
    free(record2);
}

int partition_sys(int fileToSort, int from, int to, int recordLength){
    lseek(fileToSort, (recordLength + 1)*to , SEEK_SET);
    char* pivot = (char*)calloc(recordLength + 1, sizeof(char));
    read(fileToSort, pivot, recordLength + 1);

    lseek(fileToSort, (recordLength + 1)*from , SEEK_SET);      // Na poczatek przedzialu na ktorym dzialamy
    char* iter = (char*)calloc(recordLength+1, sizeof(char));
    int i = from - 1;
    
    for(int j = from; j < to; j++){
        read(fileToSort, iter, recordLength + 1);
        if(strcmp(pivot, iter) < 0)
            swap_sys(fileToSort, ++i, j, recordLength);
    }

    i++;
    swap_sys(fileToSort, i, from, recordLength);  

    free(pivot);
    free(iter);
    return i;
}

void qsort_sys(int fileToSort, int from, int to, int recordLength){
    if(from < to){
        int mid = partition_sys(fileToSort, from, to, recordLength);
        qsort_sys(fileToSort, from, mid-1, recordLength);
        qsort_sys(fileToSort, mid+1, to, recordLength);
    }
}



void swap_lib(FILE* fileToSort, int idx1, int idx2, int recordLength){
    char* record1 = (char*) calloc(recordLength + 1, sizeof(char) );
    char* record2 = (char*) calloc(recordLength + 1, sizeof(char) );

    // Wyciagamy recordy do zamiany
    fseek(fileToSort, (recordLength + 1)*idx1, 0);
    fread(record1, sizeof(char), recordLength + 1, fileToSort);
    fseek(fileToSort, (recordLength + 1)*idx2, 0);
    fread(record2, sizeof(char), recordLength + 1, fileToSort);

    // Zapisujemy je na zamienionych pozycjach
    fseek(fileToSort, (recordLength + 1)*idx1, 0);
    fwrite(record2, sizeof(char), recordLength + 1, fileToSort);
    fseek(fileToSort, (recordLength + 1)*idx2, 0);
    fwrite(record1, sizeof(char), recordLength + 1, fileToSort);

    free(record1);
    free(record2);
}

int partition_lib(FILE* fileToSort, int from, int to, int recordLength){
    fseek(fileToSort, (recordLength + 1)*to , 0);
    char* pivot = (char*)calloc(recordLength + 1, sizeof(char));
    fread(pivot, sizeof(char), recordLength + 1, fileToSort);

    fseek(fileToSort, (recordLength + 1)*from , 0);      // Na poczatek przedzialu na ktorym dzialamy
    char* iter = (char*)calloc(recordLength+1, sizeof(char));
    int i = from - 1;
    
    for(int j = from; j < to; j++){
        fread(iter, sizeof(char), recordLength + 1, fileToSort);
        if(strcmp(pivot, iter) < 0)
            swap_lib(fileToSort, ++i, j, recordLength);
    }

    i++;
    swap_lib(fileToSort, i, from, recordLength);  

    free(pivot);
    free(iter);
    return i;
}


void qsort_lib(FILE* fileToSort, int from, int to, int recordLength){
    if(from < to){
        int mid = partition_lib(fileToSort, from, to, recordLength);
        qsort_lib(fileToSort, from, mid-1, recordLength);
        qsort_lib(fileToSort, mid+1, to, recordLength);
    }
}

void sort(const char* filename, int recordCount, int recordLength, int isLib){        // isLib > 0 => przy pomocy funkcji bibliotecznych
    if(isLib){
        FILE* fileToSort = fopen(filename, "r+");
        if (fileToSort == NULL){
            fprintf(stderr, "Podany plik do sortowania nie istnieje!\n");
            exit(1);
        }
        qsort_lib(fileToSort, 0, recordCount - 1, recordLength);    // sortowanie [0,recordCount)
        fclose(fileToSort);
    }
    else{
        int sysFileToSort = open(filename, O_RDWR);
        if (sysFileToSort < 0){
            fprintf(stderr, "Podany plik do sortowania nie istnieje!\n");
            exit(1);
        }
        qsort_sys(sysFileToSort, 0, recordCount - 1, recordLength);
        close(sysFileToSort);
    }
    printf("Poprawnie posortowano plik %s zawierajacy %d rekordow dlugosci %d w trybie %s.\n", filename, recordCount, recordLength, isLib ? "lib" : "sys");
}



void copy_sys(int sourcefile, int directionfile, int recordCount, int bufferSize){
    char* buffer = (char*)calloc(bufferSize + 1, sizeof(char)); // +1 na \n (dla kompatybilnosci)
    for(int i=0; i < recordCount; i++){
        read(sourcefile, buffer, bufferSize + 1);
        write(directionfile, buffer, bufferSize + 1);
    }
    free(buffer);    
}


void copy_lib(FILE* sourcefile, FILE* directionfile, int recordCount, int bufferSize){
    char* buffer = (char*)calloc(bufferSize + 1, sizeof(char)); // +1 na \n (dla kompatybilnosci)
    for(int i=0; i < recordCount; i++){
        fread(buffer, sizeof(char), bufferSize + 1, sourcefile);
        fwrite(buffer, sizeof(char), bufferSize + 1, directionfile);
    }
    free(buffer);  
}

// kopiowanie sourcefile -> directionfile
void copy(const char* sourcefile, const char* directionfile, int recordCount, int bufferSize, int isLib){   // isLib > 0 => przy pomocy bibliotecznych funkcji
    if(isLib){
        FILE* source = fopen(sourcefile,"r");
        if (source == NULL){
            fprintf(stderr,"Plik zrodlowy do kopiowania nie istnieje!\n");
            exit(1);
        }
        FILE* direction = fopen(directionfile, "r");
        if (direction != NULL){
            printf("Podany plik docelowy juz istnieje. Czy chcesz go nadpisac(y/n)?\n");
            char answer;
            do{
                scanf("%c",&answer);
            }while(answer != 'n' && answer != 'y');

            if(answer == 'n'){
                printf("Nie nadpisuje. Kopiowanie nie zostalo przeprowadzone.\n");
                fclose(source);
                fclose(direction);
                return;
            }
            fclose(direction);
        }
        direction = fopen(directionfile, "w");

        copy_lib(source, direction, recordCount, bufferSize);
        fclose(source);
        fclose(direction);
    }
    else{
        int source = open(sourcefile,O_RDONLY);
        if (source < 0){
            fprintf(stderr,"Plik zrodlowy do kopiowania nie istnieje!\n");
            exit(1);
        }
        int direction = open(directionfile, O_RDONLY);
        if (direction >= 0){
            printf("Podany plik docelowy juz istnieje. Czy chcesz go nadpisac(y/n)?\n");
            char answer;
            do{
                scanf("%c",&answer);
            }while(answer != 'n' && answer != 'y');

            if(answer == 'n'){
                printf("Nie nadpisuje. Kopiowanie nie zostalo przeprowadzone.\n");
                close(source);
                close(direction);
                return;
            }
            close(direction);
        }
        direction = open(directionfile, O_WRONLY|O_CREAT);
        
        copy_sys(source, direction, recordCount, bufferSize);
        close(source);
        close(direction);
    }
    printf("Poprawnie skopiowano z pliku %s do pliku %s %d rekordow buforem rozmiaru %d w trybie %s.\n", sourcefile, directionfile, recordCount, bufferSize, isLib ? "lib" : "sys");
}



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




int main(int argc, char** argv){
    // Inicjalizacja generatora liczb pseudolosowych
    time_t t;
    srand((unsigned) time(&t));

    if (argc < 2){                                                  
        printf("Nie podano argumentow!\n");                         
        exit(1);
    }

    FILE* resHandle = fopen("wyniki.txt","a");

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
    writeResults(resHandle, operation);
    fclose(resHandle);
        
    return 0;
}