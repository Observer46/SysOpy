#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

void err_exit_arg(){
    printf("execTest moze przyjmowac jako parametr: ignore/mask/pending\n");
    exit(1);
}

void is_pending(){
    sigset_t pending_sigs;
    if ( sigpending(&pending_sigs) < -1){
        fprintf(stderr,"Nie udalo sie odczytac listy sygnalow oczekujacych na odblokowanie!\n");
        exit(4);
    }
    if ( sigismember(&pending_sigs, SIGUSR1) )
        printf("Sygnal SIGUSR1 oczekuje w programie execTest o PID: %d.\n", getpid());
    else
        printf("Nie ma sygnalu SIGUSR1 posrodu oczekujacych w programie execTest o PID: %d!\n",getpid());
}

int main(int argc, char** argv){

    if (argc != 1){
        fprintf(stderr, "Podano niepoprawna ilosc parametrow dla execTest! Powinien byc 1!\n");
        err_exit_arg();
    }

    char* option = argv[0];

    if ( strcmp(option, "ignore") == 0 || strcmp(option, "mask") == 0 ){
        printf("Wysylam sygnal SIGUSR1 w execTest...\n");
        raise(SIGUSR1);
    }
    else if ( strcmp(option, "pending") == 0){
        printf("Sprawdzam, czy w execTest jest widoczny wiszacy sygnal SIGUSR1...\n");
        is_pending();
    }
    else{
        fprintf(stderr, "Nieznany parametr wywolania execTest!\n");
        err_exit_arg();
    }

    printf("\n");
    return 0;
}