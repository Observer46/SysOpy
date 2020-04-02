#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>

int received_signals = 0;
char* glob_mode = NULL;
int signal_count;

int SIG_COUNT;
int SIG_END;

void err_exit_arg(){
    printf("Sygnatura wywolania sender: catcher_PID signal_count kill|sigqueue|sigrt\n");
    exit(1);
}

void err_exit_sig_action(){     // Wyjscie gdy nie udalo sie ustawic odpowiedniej reakcji na sygnal
    fprintf(stderr, "Sender: Ustawienie sposobu przechwytywania jednego z syngalow nie powiodlo sie!\n");
    exit(2);
}

void err_exit_mask(){
    fprintf(stderr,"Sender: Nie powiodlo sie ustawienie maski!\n");
    exit(6);
}

void send_SIG_COUNT(pid_t catcher_PID){
    if ( strcmp(glob_mode, "kill") == 0 || strcmp(glob_mode, "sigrt") == 0 ){
        // kill lub sigrt
        if ( kill(catcher_PID, SIG_COUNT) < 0 ){
            fprintf(stderr,"Sender: Nie wyslalem poprawnie sygnalu za pomoca kill!\n");
            exit(2);
        }
    }
    else{
        // sigqueue
        union sigval value;
        value.sival_int = 0;        // Inicjalizujemy jakkolwiek, po ten stronie kanalu komunikacyjnego to nie ma znaczenia
        if ( sigqueue(catcher_PID, SIG_COUNT, value) < 0 ){
            fprintf(stderr,"Sender: Nie wyslalem poprawnie sygnalu za pomoca sigqueue!\n");
            exit(3);
        }
    }
}

void send_SIG_END(pid_t catcher_PID){
    if ( strcmp(glob_mode, "kill") == 0 || strcmp(glob_mode, "sigrt") == 0 ){
        // kill lub sigrt
        if ( kill(catcher_PID, SIG_END) < 0 ){
            fprintf(stderr,"Sender: Nie wyslalem poprawnie sygnalu konczacego za pomoca kill!\n");
            exit(2);
        }
    }
    else{
        // sigqueue
        union sigval value;
        value.sival_int = 0;
        if ( sigqueue(catcher_PID, SIG_END, value) < 0 ){
            fprintf(stderr,"Sender: Nie wyslalem poprawnie sygnalu konczacego za pomoca sigqueue!\n");
            exit(3);
        }
    }
}

void signal_handler(int sig, siginfo_t* sig_info, void* ucontext){
    pid_t catcher_PID = sig_info -> si_pid;
    // printf("Sender: otrzymalem sygnal - %d\n", received_signals);        // Dzieki temu mozemy sprawdzic, czy sygnaly sa otrzymywane na zmiane
    if (sig == SIG_COUNT){
        received_signals++;
        if (received_signals < signal_count){       // Jesli nie dostlismy tyle co trzeba to wysylamy nowe
            send_SIG_COUNT(catcher_PID);
        }
        else{       // Jak nie to wysylamy SIG_END
            send_SIG_END(catcher_PID);
        }   
    }
    else{
        printf("Sender: Dostalem %d sygnalow, powinno ich byc %d.\nSender: Koncze dzialanie.\n\n",received_signals, signal_count);
        exit(0);
    }
}

void prepare_handlers(int sig_count, int sig_end){
    struct sigaction action;
    if ( sigemptyset(&action.sa_mask) < 0){
        fprintf(stderr,"Sender: Nie udalo sie wyczyscic maski sygnalow w sigaction!\n");
        err_exit_sig_action();
    }

    if ( sigaddset(&action.sa_mask, sig_count) < 0 || sigaddset(&action.sa_mask, sig_end) < 0){
        fprintf(stderr,"Sender: Nie udalo sie zmienic maski sygnalow dla sigaction!\n");
        err_exit_sig_action();
    }
    
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = signal_handler;

    if ( sigaction(sig_count, &action, NULL) < 0 || sigaction(sig_end, &action, NULL) < 0 )
        err_exit_sig_action();
}

int isANumber(char* number){
    for(int i=0; i < strlen(number); i++)
        if ( !isdigit(number[i]) )
            return 0;
    return 1;
}

int main(int argc, char** argv){

    if (argc != 4){
        fprintf(stderr, "Sender: Podano niepoprawna ilosc parametrow dla programu sender! Powinien byc ich 3!\n");
        err_exit_arg();
    }

    char* PID_chr = argv[1];
    char* sig_count_chr = argv[2];
    char* sending_mode = argv[3];

    if ( !isANumber(PID_chr) ){
        fprintf(stderr,"Sender: PID catchera musi byc dodatnia liczba!\n");
        err_exit_arg();
    }
    if ( !isANumber(sig_count_chr) ){
        fprintf(stderr,"Sender: Liczba procesow do wyslania musi byc dodatnia liczba!\n");
        err_exit_arg();
    }

    pid_t catcher_PID = atoi(PID_chr);
    signal_count = atoi(sig_count_chr);

    // Jesli tryb wyslania nie jest zadnym ze znanych to konczymy dzialanie
    if ( strcmp(sending_mode, "kill") != 0 && strcmp(sending_mode, "sigqueue") != 0 && strcmp(sending_mode, "sigrt") != 0){
        fprintf(stderr,"Sender: Nieznany tryb wysylania sygnalow!\n");
        err_exit_arg();
    }


    

    glob_mode = sending_mode;

    sigset_t mask;          // Wypelnianie maski sygnalow tak, by odbierala tylko SIGUSR1 i SIGUSR2
    if ( sigfillset(&mask) < 0 )
        err_exit_mask();

    if ( strcmp(sending_mode, "sigrt") == 0){
        if( sigdelset(&mask, SIGRTMIN + 1) < 0 || sigdelset(&mask, SIGRTMIN + 2) < 0)
            err_exit_mask();
        SIG_COUNT = SIGRTMIN + 1;
        SIG_END = SIGRTMIN + 2;
    }
    else{
        if( sigdelset(&mask, SIGUSR1) < 0 || sigdelset(&mask, SIGUSR2) < 0)
            err_exit_mask();
        SIG_COUNT = SIGUSR1;
        SIG_END = SIGUSR2;
    }

    if ( sigprocmask(SIG_SETMASK, &mask, NULL) < 0 )        // Ustawienie maski
        err_exit_mask();

    prepare_handlers(SIG_COUNT, SIG_END);  

    send_SIG_COUNT(catcher_PID);
     
    while(1){
        pause();
    }
    
    return 0;
}