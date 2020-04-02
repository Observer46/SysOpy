#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

int* stop = NULL;

void change_flag(){
    *stop = 1 - *stop;
}

void handle_SIGINT(int sig){
    printf("\nOdebrano sygnal SIGINT\n");
    exit(0);
}

void handle_SIGTSTP(int sig){
    if(!*stop)
        printf("\nOczekuje na CTRL+Z - kontynuacja albo CTRL+C - zakonczenie programu\n");
    else
        printf("\n");
    change_flag();
}

int main(){
    printf("Wlaczono program main\n");
    if ( signal(SIGTSTP, handle_SIGTSTP) == SIG_ERR ){
        fprintf(stderr, "Nie udalo sie ustawic handlera dla SIGTSTP!\n");
        exit(1);
    }

    struct sigaction action;
    action.sa_handler = handle_SIGINT;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    if ( sigaction(SIGINT, &action, NULL) == -1 ){
        fprintf(stderr, "Nie udalo sie ustawic handlera dla SIGINT!\n");
        exit(2);
    }

    int stop_flag = 0;
    stop = &stop_flag;

    while(1){
        if(stop_flag)
            pause();
        system("ls ");
        sleep(1);
    }
    return 0;
}
