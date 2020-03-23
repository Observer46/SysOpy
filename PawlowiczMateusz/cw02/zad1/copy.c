#include "copy.h"

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
        direction = open(directionfile, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        
        copy_sys(source, direction, recordCount, bufferSize);
        close(source);
        close(direction);
    }
    printf("Poprawnie skopiowano z pliku %s do pliku %s %d rekordow buforem rozmiaru %d w trybie %s.\n", sourcefile, directionfile, recordCount, bufferSize, isLib ? "lib" : "sys");
}