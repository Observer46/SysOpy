//#include <stdio.h>
//#include <stdlib.h>
#include <time.h>
//#include <sys/sem.h>
//#include <sys/shm.h>
#include <signal.h>
// #include <sys/types.h>
// #include <sys/ipc.h>
#include <sys/wait.h>
#include <unistd.h>

#include "systemV_shop_settings.h"
#include "utils.h"

// Globalne ze wzgledu na potrzebe korzystania z nich przy sygnale wyjscia
pid_t children_pids[TOTAL_WORKERS];
int semaphores = -1;
int shared_memory = -1;

int exit_status = 0;

// Organizacja wyjscia z programu
////////////////////////////
void exit_cleanup(){
    if(semaphores != -1 && semctl(semaphores, 0, IPC_RMID, NULL) == -1){
        fprintf(stderr,"Main: Nie udalo sie usunac semaforow z pamieci!\n");
        if (exit_status == 0)   exit_status = 1;
    }

    if(shared_memory != -1 && shmctl(shared_memory, IPC_RMID, NULL) == -1){
        fprintf(stderr,"Main: Nie udalo sie usunac pamieci wspoldzielonej!\n");
        if (exit_status == 0)   exit_status = 2;
    }

    exit(exit_status);
}

void sigint_handle(int sig){
    printf("Sklep wysylkowy sie wlasnie zamyka...\n");
    for(int i=0; i < TOTAL_WORKERS; i++)
        kill(children_pids[i], SIGINT);     
    exit_cleanup();
}
////////////////////////////

// Tworzenie semaforow oraz pamieci wspoldzielonej
////////////////////////////
void create_semaphores(){
    key_t sem_key = ftok(getenv("HOME"), 'a');
    semaphores = semget(sem_key, SEMAPHORE_COUNT, IPC_CREAT|0666);
    if (semaphores == -1){
        fprintf(stderr,"Main: Nie udalo sie utworzyc zbioru semaforow!\n");
        exit(1);
    }

    union semun arg;
    arg.val = 0;        
    for(int i=0; i < SEMAPHORE_COUNT; i++)  // Inicjalizacja wszystkich semaforow
        if ( semctl(semaphores, i, SETVAL, arg) == -1 ){     // Kolejne semafory maja odpowiednie funkcje (opisane w utils.h)
            fprintf(stderr, "Main: Nie powiodlo sie ustawienie wartosci semafora!\n");
            exit_status = 1;
            exit_cleanup();
        }
}           


void create_shared_mem(){
    key_t shm_key = ftok(getenv("HOME"), 'b');
    shared_memory = shmget(shm_key, sizeof(orders_struct), IPC_CREAT|0666);
    if (shared_memory == -1){
        fprintf(stderr,"Main: Nie udalo sie utworzyc zbioru semaforow!\n");
        exit(1);
    }
}

void launch_particular_workers(const char* worker_type, int start_idx, int worker_count){
    pid_t worker_pid;
    char program_name[16];
    sprintf(program_name, "./%s", worker_type);
    for(int i=0; i < worker_count; i++){
        worker_pid = fork();
        if (worker_pid == -1){
            fprintf(stderr,"Main: fork() - nie udalo sie utworzy procesu potomnego dla %s!\n", worker_type);
            exit_status = 4;
            exit_cleanup();
        }
        if (worker_pid == 0){
            execl(program_name,worker_type,NULL);
            // Jesli uruchomienie sie nie powiedzie
            fprintf(stderr,"Main: Wystapil problem z uruchomienie programu %s!\n", worker_type);
            exit_status = 4;
            exit_cleanup();
        }
        else{
            printf("Sklep wysylkowy: Przybyl %s %d\n", worker_type, i);
            children_pids[start_idx + i] = worker_pid;
            sleep(1);           // Dbamy o to, aby ich seedy losowych liczb byly rozne
        }
    }
}

void deploy_workers(){
    launch_particular_workers("receiver", 0, RECEIVER_COUNT);
    //launch_particular_workers("wrapper", RECEIVER_COUNT, WRAPPER_COUNT);
    //launch_particular_workers("sender", RECEIVER_COUNT + WRAPPER_COUNT, SENDER_COUNT);
    printf("\nSklep wysylkowy: Przybyli wszyscy pracownicy - zmiana rozpoczeta!\n");
}



int main(){
    srand(time(NULL));
    create_semaphores();
    create_shared_mem();
    if (signal(SIGINT, sigint_handle) == SIG_ERR){
        fprintf(stderr,"Nie udalo sie ustawic obslugi sygnalu SIGINT!\n");
        exit(3);
    }

    deploy_workers();
    for(int i=0; i < TOTAL_WORKERS; i++)        // Czekanie, az zakoncza dzialanie
        wait(NULL);
    
    exit_cleanup();

    //printf("%d\n", rand_package());
    return 0;
}