#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

void err_exit_args(){       // Wyjscie zwiazane z niepoprawnymi parametrami programu
    printf("Mozliwe opcje wywolania: ignore/handler/mask/pending fork/exec\n");
    exit(1);
}

void err_exit_signal(){     // Wyjscie gdy nie udalo sie ustawic odpowiedniej reakcji na sygnal
    fprintf(stderr, "Ustawienie trybu przechwycenia sygnalu SIGUSR1 nie powiodlo sie!\n");
    exit(2);
}

void handler_SIGUSR1(int sig){  
    printf("Odebralem sygnal SIGUSR1 w procesie %d!\n", getpid() );
}

void is_pending(){
    sigset_t pending_sigs;
    if ( sigpending(&pending_sigs) < -1){
        fprintf(stderr,"Nie udalo sie odczytac listy sygnalow oczekujacych na odblokowanie!\n");
        exit(4);
    }
    if ( sigismember(&pending_sigs, SIGUSR1) )
        printf("Sygnal SIGUSR1 oczekuje w procesie %d.\n", getpid());
    else
        printf("Nie ma sygnalu SIGUSR1 posrodu oczekujacych w procesie %d!\n",getpid());
}

void change_action_for_SIGUSR1(char* action_for_signal){        // Ustawnienie odpowiedniej reakcji w zaleznosci od parametru wywolania
    if ( strcmp(action_for_signal, "ignore") == 0 ){
        if ( signal(SIGUSR1, SIG_IGN) == SIG_ERR )
            err_exit_signal();

    }
    else if ( strcmp(action_for_signal, "handler") == 0 ){
        struct sigaction action;
        action.sa_handler = handler_SIGUSR1;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;

        if ( sigaction(SIGUSR1, &action, NULL) == -1)
            err_exit_signal();
    }
    else if ( strcmp(action_for_signal, "mask") == 0 || strcmp(action_for_signal, "pending") == 0 ){
        sigset_t newmask;
        sigemptyset(&newmask);
        sigaddset(&newmask, SIGUSR1);
        if ( sigprocmask(SIG_BLOCK, &newmask, NULL) < 0)
            err_exit_signal();
    }
    else{
        fprintf(stderr,"Nieznana opcja wywolania!\n");
        err_exit_args();
    }
}

int main(int argc, char** argv){
    if (argc != 3){
        fprintf(stderr,"Prosze podac dokladnie dwa parametry wywolania!\n");
        err_exit_args();
    }

    char* action_for_signal = argv[1];
    char* mode = argv[2];
    change_action_for_SIGUSR1(action_for_signal);
    
    printf("\t Test dla konfiguracji: %s %s\n", action_for_signal, mode);
    printf("Wysylam sygnal...\n");
    raise(SIGUSR1);

    if ( strcmp(action_for_signal, "pending") == 0 )
        is_pending();

    if( strcmp(mode,"fork") == 0){
        // fork
        pid_t rel_pid; 
        if ( (rel_pid = fork()) == -1){
            fprintf(stderr,"Nie udalo sie utworzyc procesu potomonego przy pomocy fork!\n");
            exit(3);
        }
        if (rel_pid == 0){
            if ( strcmp(action_for_signal, "pending") == 0 )
                is_pending();
            else{
                printf("Wysylam sygnal w procesie potomnym...\n");
                raise(SIGUSR1);
            }
        }
        else
            wait(0);
    }
    else if ( strcmp(mode,"exec") == 0){
        // exec
        execl("./execTest",action_for_signal, NULL);
        fprintf(stderr,"Nie udalo sie wywolac funkcji exec w main!\n");
        exit(4);
    }
    else{
        fprintf(stderr,"Nieznany tryb testowania!\n");
        err_exit_args();
    }
    printf("\n");

    return 0;
}
