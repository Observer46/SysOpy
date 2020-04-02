#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

void err_exit_args(){
    printf("Dostepne parametry wywolania: div/child/child_int\n");
    exit(1);
}
void err_exit_signal(){
    printf("Nie udalo sie ustawic handlera dla sygnalu!\n");
    exit(2);
}

void handler_SIGFPE(int sig, siginfo_t* sig_info, void* ucontext){
    printf("Numer sygnalu: %d\n", sig_info -> si_signo);
    printf("Wystapil wyjatek arytmetyczny\n");
    printf("Adres wystapienia wyjatku: %p\n", sig_info -> si_addr);
    if (sig_info -> si_code == FPE_INTDIV)
        printf("Doszlo do calkowitoliczbowego dzielenia przez 0!\n");
    printf("\n");
    exit(0);    // Takie wyjsce, aby make nie wypisywal bledu przy tescie
    //exit(sig_info -> si_code);  // Wyjscie z kodem oznaczajacym rodzaj bledu, tutaj: FPE_INTDIV - calkowitoliczbowe dzielenie przez zero
}

void handler_SIGCHLD(int sig, siginfo_t* sig_info, void* ucontext){
    printf("Numer sygnalu: %d\n", sig_info -> si_signo);
    if (sig_info -> si_code == CLD_EXITED){         // Zwykle wyjscie exit()
        printf("Proces potomny zakonczyl dzialanie poprawnie.\n");
        printf("PID procesu potomnego: %d\n", sig_info -> si_pid);
        printf("UID uzytkownika wywolujacego: %d\n", sig_info -> si_uid);
        printf("Status wyjscia: %d\n",sig_info -> si_status);
    }
    else if (sig_info -> si_code == CLD_KILLED)     // Zabicie SIGKILL
        printf("Proces potomny zostal zabity!\n");
    else
        printf("Procesowi potomnemu przydarzylo sie cos innego niz zabicie lub poprawne zakonczenie\n");
}

int main(int argc, char** argv){
    if (argc != 2){
        fprintf(stderr,"Prosze podac dokladnie dwa parametry wywolania!\n");
        err_exit_args();
    }

    char* mode = argv[1];

    printf("\tScenariusz: %s\n", mode);

    struct sigaction action;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO;


    if ( strcmp(mode, "div") == 0 ){
        action.sa_sigaction = handler_SIGFPE;
        if ( sigaction(SIGFPE, &action, NULL) == -1 )
            err_exit_signal();
        int one = 1;
        int zero = 0;
        printf("%d\n", one % zero);     // Tu bedzie blad i zostanie wyslany sygnal SIGFPE
    }
    else if ( strcmp(mode, "child") == 0 || strcmp(mode, "child_int") == 0 ){
        action.sa_sigaction = handler_SIGCHLD;
        if ( sigaction(SIGCHLD, &action, NULL) == -1 )
            err_exit_signal();

        pid_t rel_pid = fork();
        if(rel_pid == 0){
            if ( strcmp(mode, "child") == 0 ){
                int status = 42;
                printf("Ja, proces potomny o PID: %d wychodze z programu z kodem statusu: %d.\n", getpid(), status);
                exit(status);       
            }
            else
                raise(SIGKILL);
        }
        else{
            wait(0);
        }
    }
    else{
        fprintf(stderr,"Nieznana opcja wywolania!\n");
        err_exit_args();
    }

    printf("\n");
    return 0;
}
