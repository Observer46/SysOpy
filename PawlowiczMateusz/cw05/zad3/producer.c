#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

void error_exit_args(){
    fprintf(stderr,"Producer: potrzebne argumenty to: potok_nazwany plik_tekstowy N(liczba znakow w jednorazowym czytaniu pliku) \n");
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
        fprintf(stderr,"Producer: N (liczba znakow czytach w jednorazowym odczycie z pliku) musi byc dodatnia liczba!\n");
        exit(2);
    }

    int N = atoi(chr_count);

    time_t t;
    srand( (unsigned) time(&t) );
    
    int read_file_fd = open(file_path, O_RDONLY);
    if (read_file_fd == -1){
        fprintf(stderr,"Producer: Nie udalo sie otworzyc pliku wejscowego %s!\n",file_path);
        exit(2);
    }
    int named_pipe_fd = open(named_pipe, O_WRONLY|O_APPEND);

    
    char* buffer = (char*)calloc(N+1, sizeof(char));    // +1 dla bezpieczenstwa
    char* data_to_pipe = (char*)calloc(N+8, sizeof(char));    // +7 dla formatu #PID# (zakladam, ze PID ma max. 5 cyfr), +1 dla '\n'
    int byte_count = read(read_file_fd, buffer, N);
    char mypid [7] = "";
    sprintf(mypid,"#%d#",getpid());

    while ( byte_count > 0 ){    // Pobieramy N znakow
        int sleep_time = rand() % 2 + 1;    // Spimy 1 lub 2 sekundy
        sleep(sleep_time);
        
        sprintf(data_to_pipe,"%s%s\n", mypid, buffer);
        int data_len = byte_count + strlen(mypid)+1;
        data_to_pipe[data_len] = '\0';
        printf("Producer: Wysylam %s",data_to_pipe);
        write(named_pipe_fd, data_to_pipe, data_len); // Wysylamy potokiem N znakow wraz z PIDem tego procesu

        byte_count = read(read_file_fd, buffer, N);
        buffer[byte_count] = '\0';      // Zaznaczamy koniec odczytanego kawalka
    }

    free(buffer);
    free(data_to_pipe);

    if( close(named_pipe_fd) == -1 ){
        fprintf(stderr,"Producer: Nie udalo sie poprawnie zamknac potoku nazwanego %s!\n",named_pipe);
        exit(3);
    }
    if( close(read_file_fd) == -1 ){
        fprintf(stderr,"Producer: Nie udalo sie poprawnie zamknac pliku wejsciowego %s!\n",file_path);
        exit(3);
    }


    return 0;
}