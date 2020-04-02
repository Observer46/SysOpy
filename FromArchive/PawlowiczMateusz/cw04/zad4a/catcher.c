
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int caught_signals = 0;
char* sending_mode = NULL;

int SIG_COUNT;
int SIG_END;

void err_exit_args(){       // Wyjscie zwiazane z niepoprawnymi parametrami programu
    printf("Catcher: Mozliwe opcje wywolania: kill|sigqueue|sigrt\n");
    exit(1);
}

void err_exit_sig_action(){     // Wyjscie gdy nie udalo sie ustawic odpowiedniej reakcji na sygnal
    fprintf(stderr, "Catcher: Ustawienie sposobu przechwytywania jednego z syngalow nie powiodlo sie!\n");
    exit(2);
}

void err_exit_mask(){
    fprintf(stderr,"Catcher: Blad zwiazany z maska blokujaca sygnaly!\n");
    exit(3);
}

void signal_handler(int sig, siginfo_t* sig_info, void* ucontext){
    if (sig == SIG_COUNT)    caught_signals++;
    else{
        int sender_PID = sig_info -> si_pid;
        if ( strcmp(sending_mode, "kill") == 0 || strcmp(sending_mode, "sigrt") == 0){
            // kill lub sigrt
            for (int i=0; i < caught_signals; i++){
                if ( kill(sender_PID, SIG_COUNT) < 0 ){
                    fprintf(stderr,"Catcher: Nie wyslalem poprawnie sygnalu za pomoca kill!\n");
                    exit(2);
                }
            }
            if ( kill(sender_PID, SIG_END) < 0 ){
                fprintf(stderr,"Catcher: Nie wyslalem poprawnie sygnalu konczacego za pomoca kill!\n");
                exit(2);
            }
        }
        else{
            // sigqueue
            union sigval value;
            for (int i=0; i < caught_signals; i++){
                value.sival_int = i; 
                if ( sigqueue(sender_PID, SIG_COUNT, value) < 0 ){
                    fprintf(stderr,"Catcher: Nie wyslalem poprawnie sygnalu za pomoca sigqueue!\n");
                    exit(3);
                }
            }
            if ( sigqueue(sender_PID, SIG_END, value) < 0 ){
                fprintf(stderr,"Catcher: Nie wyslalem poprawnie sygnalu konczacego za pomoca sigqueue!\n");
                exit(3);
            }
        }
        printf("Catcher: Odebralem %d sygnalow %s.\nCatcher: Koncze dzialanie.\n\n", caught_signals, ( (strcmp(sending_mode, "sigrt") == 0) ? "SIGRTMIN" : "SIGUSR1"));
        exit(0);
    }
}

void prepare_handlers(int sig_count, int sig_end){
    struct sigaction action;
    if ( sigemptyset(&action.sa_mask) < 0){
        fprintf(stderr,"Catcher: Nie udalo sie wyczyscic maski sygnalow w sigaction!\n");
        err_exit_sig_action();
    }

    if ( sigaddset(&action.sa_mask, sig_count) < 0 || sigaddset(&action.sa_mask, sig_end) < 0){
        fprintf(stderr,"Catcher: Nie udalo sie zmienic maski sygnalow dla sigaction!\n");
        err_exit_sig_action();
    }
    
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = signal_handler;

    if ( sigaction(sig_count, &action, NULL) < 0 || sigaction(sig_end, &action, NULL) < 0 )
        err_exit_sig_action();
}

int main(int argc, char** argv){
    if (argc != 2){
        fprintf(stderr,"Catcher: Niepoprawna ilosc parametrow! Sender przyjmuje tylko jeden!\n");
        err_exit_args();
    }


    sending_mode = argv[1];
    if ( strcmp(sending_mode, "kill") != 0 && strcmp(sending_mode, "sigqueue") != 0 && strcmp(sending_mode, "sigrt") != 0){
        fprintf(stderr,"Catcher: Nieznany tryb wysylania sygnalow!\n");
        err_exit_args();
    }


    sigset_t mask;          // Wypelnianie maski sygnalow tak, by odbierala tylko SIGUSR1 i SIGUSR2
    if ( sigfillset(&mask) < 0 )
        err_exit_mask();

    if ( strcmp(sending_mode, "sigrt") == 0){           // Ustawienie maski by blokowala wszystkie poza wybranymi dwoma
        if( sigdelset(&mask, SIGRTMIN) < 0 || sigdelset(&mask, SIGRTMIN + 1) < 0)
            err_exit_mask();
        SIG_COUNT = SIGRTMIN;
        SIG_END = SIGRTMIN + 1;
    }
    else{
        if( sigdelset(&mask, SIGUSR1) < 0 || sigdelset(&mask, SIGUSR2) < 0)
            err_exit_mask();
        SIG_COUNT = SIGUSR1;
        SIG_END = SIGUSR2;
    }

    if ( sigprocmask(SIG_SETMASK, &mask, NULL) < 0 )        // Wlaczenie maski
        err_exit_mask();

    prepare_handlers(SIG_COUNT, SIG_END);

    printf("Catcher: Moj PID: %d\n", getpid());
    while (1){
        pause();
    }

    
    return 0;
}
