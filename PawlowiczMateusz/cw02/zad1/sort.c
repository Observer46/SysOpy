#include "sort.h"

void swap_sys(int fileToSort, int idx1, int idx2, int recordLength, char* recordBuf1, char* recordBuf2){
    //char* record1 = (char*) calloc(recordLength + 1, sizeof(char));   // Bo chcemy miec tylko 2 naraz w pamieci
    //char* record2 = (char*) calloc(recordLength + 1, sizeof(char));

    // Wyciagamy recordy do zamiany
    lseek(fileToSort, (recordLength + 1)*idx1, SEEK_SET);
    read(fileToSort, recordBuf1, recordLength + 1);
    lseek(fileToSort, (recordLength + 1)*idx2, SEEK_SET);
    read(fileToSort, recordBuf2, recordLength + 1);

    // Zapisujemy je na zamienionych pozycjach
    lseek(fileToSort, (recordLength + 1)*idx1, SEEK_SET);
    write(fileToSort, recordBuf2, recordLength + 1);
    lseek(fileToSort, (recordLength + 1)*idx2, SEEK_SET);
    write(fileToSort, recordBuf1, recordLength + 1);

    //free(record1);
    //free(record2);
}

int partition_sys(int fileToSort, int from, int to, int recordLength){
    lseek(fileToSort, (recordLength + 1)*to , SEEK_SET);
    char* pivot = (char*)calloc(recordLength + 1, sizeof(char));
    read(fileToSort, pivot, recordLength + 1);

    lseek(fileToSort, (recordLength + 1)*from , SEEK_SET);      // Na poczatek przedzialu na ktorym dzialamy
    char* iter = (char*)calloc(recordLength+1, sizeof(char));
    int i = from - 1;
    
    for(int j = from; j < to; j++){
        lseek(fileToSort, (recordLength + 1)*j , SEEK_SET);
        read(fileToSort, iter, recordLength + 1);
        if(strcmp(pivot, iter) > 0){
            swap_sys(fileToSort, ++i, j, recordLength, iter, pivot);
            lseek(fileToSort, (recordLength + 1)*to , SEEK_SET);
            read(fileToSort, pivot, recordLength + 1);          // pivot z poworotem jest pivotem
        }
    }

    i++;
    swap_sys(fileToSort, i, to, recordLength, iter, pivot);  

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



void swap_lib(FILE* fileToSort, int idx1, int idx2, int recordLength, char* recordBuf1, char* recordBuf2){
    //char* record1 = (char*) calloc(recordLength + 1, sizeof(char) );  // Bo chcemy miescic sie tylko w dwoch char* naraz w pamieci
    //char* record2 = (char*) calloc(recordLength + 1, sizeof(char) );

    // Wyciagamy recordy do zamiany
    fseek(fileToSort, (recordLength + 1)*idx1, 0);
    fread(recordBuf1, sizeof(char), recordLength + 1, fileToSort);
    fseek(fileToSort, (recordLength + 1)*idx2, 0);
    fread(recordBuf2, sizeof(char), recordLength + 1, fileToSort);

    // Zapisujemy je na zamienionych pozycjach
    fseek(fileToSort, (recordLength + 1)*idx1, 0);
    fwrite(recordBuf2, sizeof(char), recordLength + 1, fileToSort);
    fseek(fileToSort, (recordLength + 1)*idx2, 0);
    fwrite(recordBuf1, sizeof(char), recordLength + 1, fileToSort);

    //free(record1);
    //free(record2);
}

int partition_lib(FILE* fileToSort, int from, int to, int recordLength){
    char* pivot = (char*)calloc(recordLength + 1, sizeof(char));
    fseek(fileToSort, (recordLength + 1)*to , 0);
    fread(pivot, sizeof(char), recordLength + 1, fileToSort);

    
    char* iter = (char*)calloc(recordLength+1, sizeof(char));
    int i = from - 1;
    //fseek(fileToSort, (recordLength + 1)*from , 0);      // Na poczatek przedzialu na ktorym dzialamy (nie wystarcza)
    for(int j = from; j < to; j++){
        fseek(fileToSort, (recordLength + 1)*j , 0);
        fread(iter, sizeof(char), recordLength + 1, fileToSort);
        if(strcmp(pivot, iter) > 0){
            swap_lib(fileToSort, ++i, j, recordLength, iter, pivot);
            fseek(fileToSort, (recordLength + 1)*to , 0);
            fread(pivot, sizeof(char), recordLength + 1, fileToSort);   // pivot z powrotem pivotem
        }
        
        
    }

    i++;
    swap_lib(fileToSort, i, to, recordLength, iter, pivot);  

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