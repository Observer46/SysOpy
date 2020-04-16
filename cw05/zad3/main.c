#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

void create_input_for_producer(char* filename, int* free_chars, int producer_N){
    FILE* new_file = fopen(filename,"w");

    char file_filler = rand() % 94 + 33;
    while ( !free_chars[(size_t)file_filler] )
        file_filler = rand() % 94 + 33;

    int content_counter = rand() % 6 + 5;
    char* content = (char*)calloc(producer_N, sizeof(char) );
    for(int i=0; i < producer_N; i++)
        content[i] = file_filler;

    for(int i=0; i < content_counter; i++)
        fwrite(content, sizeof(char), producer_N, new_file);

    free(content);
    fclose(new_file);
}

int main(int argc, char** argv){
    if (argc != 6){
        fprintf(stderr,"Wymagane argumenty to: nazwa_potoku liczba_producentow N_producenta N_konsumenta output_name\n");
        exit(1);
    }
    time_t t;
    srand((unsigned) time(&t));
    char* chr_producer_N = argv[3];
    char* chr_consumer_N = argv[4];
    //////////////////////
    // Sprawdzanie arguemtnow
    //////////////////////

    char* pipe_name = argv[1];
    int producer_count = atoi(argv[2]);     // W testach 5
    int producer_N = atoi(chr_producer_N);
    char* output_filename = argv[5];

    if ( mkfifo(pipe_name, 0644) == -1 ){
        fprintf(stderr,"Nie udalo sie stworzyc potoku nazwanego: %s\n",pipe_name);
        exit(2);
    }

    char name_template[20] = "input_producer";

    int free_chars[256];        // Pomaga wypelniac tablice tak, by byly rozroznialne
    for(int i=0; i < 256; i++)
        free_chars[i] = 1;

    for(int i=0; i < producer_count; i++){
        char filename[20] = "";
        sprintf(filename,"%s%d", name_template, i);
        create_input_for_producer(filename, free_chars, producer_N);
    }
    
    pid_t pid;
    for(int i=0; i < producer_count; i++){
        char filename[20] = "";
        sprintf(filename,"%s%d", name_template, i);

        pid = fork();
        if (pid == 0){
            execl("./producer","./producer", pipe_name, filename, chr_producer_N, NULL);
            fprintf(stderr,"Nie udalo sie poprawnie uruchomic programu ./producer\n");
            exit(3);
        }
        sleep(1);           // Aby dostaly rozne seedy dla rand()
    }

    pid = fork();
    if(pid == 0){
        execl("./consumer","./consumer", pipe_name, output_filename, chr_consumer_N, NULL);
        fprintf(stderr,"Nie udalo sie poprawnie uruchomic programu ./consumer\n");
        exit(4);
    }


    for(int i=0; i < producer_count; i++)   // Czekamy na kazdy proces potomny
        wait(NULL);

    return 0;
}