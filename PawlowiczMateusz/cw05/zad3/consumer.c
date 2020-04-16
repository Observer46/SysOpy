#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

void error_exit_args(){
    fprintf(stderr,"Consumer: potrzebne argumenty to: potok_nazwany plik_wyjsciowy N(liczba znakow w jednorazowym czytaniu z potoku) \n");
    exit(1);
}

int isANumber(char* number){
    for(int i=0; i < strlen(number); i++)
        if ( !isdigit(number[i]) )
            return 0;
    return 1;
}

int main(int argc, char** argv){

    if (argc != 4)
        error_exit_args();
    
    char* named_pipe = argv[1];
    char* file_path = argv[2];
    char* chr_count = argv[3];

    if ( !isANumber(chr_count) && atoi(chr_count) > 0){
        fprintf(stderr,"Consumer: N (liczba znakow czytach w jednorazowym odczycie z potoku) musi byc dodatnia liczba!\n");
        exit(2);
    }

    int N = atoi(chr_count);

    int named_pipe_fd = open(named_pipe,O_RDONLY);
    if (named_pipe_fd == -1){
        fprintf(stderr,"Consumer: Nie udalo sie otworzyc potoku nazwanego %s!\n",named_pipe);
        exit(2);
    }
    
      
    int output_file = open(file_path,O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);

    char* buffer = (char*)calloc(N+1, sizeof(char));    // +1 dla bezpieczenstwa
    int byte_count = read(named_pipe_fd, buffer, N);
    while ( byte_count > 0 ){       // Pobieramy N znakow
        write(output_file, buffer, byte_count);          // Zapisujemy do pliku N znakow
        byte_count = read(named_pipe_fd, buffer, N);
    }
    

    free(buffer);

    if( close(named_pipe_fd) == -1){
        fprintf(stderr,"Consumer: Nie udalo sie poprawnie zamknac potoku nazwanego %s!\n",named_pipe);
        exit(3);
    }
    if( close(output_file) == -1){
        fprintf(stderr,"Consumer: Nie udalo sie poprawnie zamknac pliku wejsciowego %s!\n",file_path);
        exit(3);
    }


    return 0;
}