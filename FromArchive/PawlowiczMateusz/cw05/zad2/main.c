#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv){
    const int maxLineLength = 8192;     // Dla uproszczenia, by nie biegac po calej linijce i nie liczyc znakow, zeby potem callocowac

    if (argc != 2){
        fprintf(stderr,"Prosze podac tylko jeden argument - sciezke pliku do posorotwania\n");
        exit(1);
    }

    char* filepath = argv[1];
    FILE* commandFile = fopen(filepath, "r");
    if (commandFile == NULL){
        fprintf(stderr,"Nie udalo sie otworzyc pliku: %s\n",filepath);
        exit(2);
    }

    char line_from_file[maxLineLength];
    strcpy(line_from_file,"");
    FILE* sort  = popen("sort", "w");      // Bedziemy wysylac plik potokiem
    
    while ( !feof(commandFile) ){
        fgets(line_from_file, maxLineLength, commandFile);
        fwrite(line_from_file, sizeof(char), strlen(line_from_file), sort);     // Linijka po linijce wysylamy potokiem do sort
    }

    if ( pclose(sort) == -1){
        fprintf(stderr, "Nie udalo sie poprawnie zamknac potoku!\n");
        exit(3);
    }
    if ( fclose(commandFile) == EOF){
        fprintf(stderr, "Nie udalo sie poprawnie zamknac pliku wejsciowego!\n");
        exit(4);
    }

    return 0;
}