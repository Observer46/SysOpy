#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void err_exit_fork(){
    fprintf(stderr,"Main: Nie udalo sie podzielic wywolanie fork()!\n");
    exit(1);
}

int main(int argc, char** argv){
    pid_t catcher, sender;
    
    if (argc < 3){
        fprintf(stderr,"Automatyczne uruchamianie wymaga: liczby_sygnalow kill|sigqueue|sigrt\n");
        exit(2);
    }

    printf("\n\tSygnaly: %s, tryb: %s\n",argv[1], argv[2]);

    catcher = fork();
    if (catcher == 0){
        execl("./catcher", "./catcher", argv[2], NULL);
        fprintf(stderr,"Main: nie udalo sie uruchomic poprawnie Catchera!\n");
        exit(3);
    }
    else{
        usleep(500);
        sender = fork();
        if (sender == 0){
            char catcher_pid[6];
            sprintf(catcher_pid,"%d", catcher);
            execl("./sender", "./sender", catcher_pid, argv[1], argv[2], NULL);
            fprintf(stderr,"Main: nie udalo sie uruchomic poprawnie Sender!\n");
            exit(3);
        }
        else{
            wait(0);
        }

        wait(0);
    }
    

    return 0;
}