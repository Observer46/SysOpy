#include "generate.h"

char* create_record(int length){
    char* record = (char*) calloc(length + 1,sizeof(char));     // +1 dla \n
    for(int i=0; i < length; i++)
        record[i] = rand() % 95 + 32;       // Pozwala, by znaki byly czytelne
    record[length] = '\n';
    return record;
}


void generate(const char* filename, int recordCount, int recordLength){
    // Sprawdzamy, czy plik juz istnieje
    
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