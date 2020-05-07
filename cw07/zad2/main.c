#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include "utils.h"

// Globalne ze wzgledu na potrzebe korzystania z nich przy sygnale wyjscia
pid_t children_pids[TOTAL_WORKERS];
int shm_fd = -1;

int exit_status = 0;

// Organizacja wyjscia z programu
////////////////////////////
void exit_cleanup(){
    for (int i=0; i < SEMAPHORE_COUNT; i++)
        if(sem_unlink(semaphores_names[i]) == -1){
            fprintf(stderr,"Main: Nie udalo sie ktoregos semafora z pamieci!\n");
            if (exit_status == 0)   exit_status = 1;
        }

    if(shm_fd != -1 && shm_unlink(SHARED_MEM) == -1){
        fprintf(stderr,"Main: Nie udalo sie usunac pamieci wspoldzielonej!\n");
        if (exit_status == 0)   exit_status = 2;
    }

    exit(exit_status);
}

void sigint_handle(int sig){
    printf("\nSklep wysylkowy sie wlasnie zamyka...\n");
    for(int i=0; i < TOTAL_WORKERS; i++)
        kill(children_pids[i], SIGINT);     
    exit_cleanup();
}
////////////////////////////

// Tworzenie semaforow oraz pamieci wspoldzielonej
////////////////////////////
void create_semaphores(){   
    sem_t * sem = sem_open(semaphores_names[ARRAY_STATUS], O_RDWR|O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO, 1); // Inicjujemy jako wolna 
    if ( sem == (void*) -1 ){  
        fprintf(stderr, "Main: Nie powiodlo sie utworzenie semafora!\n");
        exit_status = 1;
        exit_cleanup();
    }
    if (sem_close(sem) == -1){
        fprintf(stderr,"Main: Nie udalo sie zamknac semafora po jego utworzeniu!\n");
        exit_status = 1;
        exit_cleanup();
    }   
    for(int i=1; i < SEMAPHORE_COUNT; i++){                 // Tworzenie i inicjalizacja wszystkich semaforow
        sem = sem_open(semaphores_names[i], O_RDWR|O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO, 0); 
        if ( sem == (void*) -1 ){    // Kolejne semafory maja odpowiednie funkcje (opisane w utils.h)
            fprintf(stderr, "Main: Nie powiodlo sie utworzenie semafora!\n");
            exit_status = 1;
            exit_cleanup();
        }
        if (sem_close(sem) == -1){
            fprintf(stderr,"Main: Nie udalo sie zamknac semafora po jego utworzeniu!\n");
            exit_status = 1;
            exit_cleanup();
        }
    }
}           


void create_shared_mem(){
    shm_fd = shm_open(SHARED_MEM, O_RDWR|O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO);
    if (shm_fd == -1 ){
        fprintf(stderr, "Main: Nie udalo sie utworzy pamieci wspoldzielonej!\n");
        exit_status = 2;
        exit_cleanup();
    }
    if ( ftruncate(shm_fd, sizeof(orders_struct)) == -1 ){
        fprintf(stderr, "Main: Nie udalo sie ustawic rozmiaru pamieci wspoldzielonej!\n");
        exit_status = 2;
        exit_cleanup();
    }
}
////////////////////////////

// Uruchamianie pracownikow
////////////////////////////
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
            fprintf(stderr,"Main: Wystapil problem z uruchomieniem programu %s!\n", worker_type);
            exit_status = 4;
            exit_cleanup();
        }
        else{
            printf("Sklep wysylkowy: Przybyl %s %d\n", worker_type, i);
            children_pids[start_idx + i] = worker_pid;
            sleep(1);           // Dbamy o to, aby ich seedy losowych liczb byly rozne
        }                       // W ten sposob zawsze zaczniemy z przepelniona tablica (co mialo byc zagwarantowane)
    }
}

void deploy_workers(){
    launch_particular_workers("receiver", 0, RECEIVER_COUNT);
    launch_particular_workers("wrapper", RECEIVER_COUNT, WRAPPER_COUNT);
    launch_particular_workers("sender", RECEIVER_COUNT + WRAPPER_COUNT, SENDER_COUNT);
    printf("\nSklep wysylkowy: Przybyli wszyscy pracownicy - zmiana rozpoczeta!\n");
}
////////////////////////


int main(){
    srand(time(NULL));
    create_semaphores();
    if (signal(SIGINT, sigint_handle) == SIG_ERR){
        fprintf(stderr,"Nie udalo sie ustawic obslugi sygnalu SIGINT!\n");
        exit(3);
    }
    create_shared_mem();
    deploy_workers();
    for(int i=RECEIVER_COUNT; i < TOTAL_WORKERS; i++)   // Po tym, jak juz wszyscy "przyszli do pracy", dajemy znak by 
        kill(children_pids[i], SIGUSR1);                // wrapperzy i senderzy zaczeli dzialac

    for(int i=0; i < TOTAL_WORKERS; i++)        // Czekanie, az zakoncza dzialanie
        wait(NULL);
    
    exit_cleanup();
}