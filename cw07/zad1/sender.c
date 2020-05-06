
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include "utils.h"

void error_exit(const char* msg, int code){
    fprintf(stderr,"%s\n",msg);
    exit(code);
}

void send_order(int semaphores, int shared_mem){
    struct sembuf* sem_ops = (struct sembuf*)calloc(4, sizeof(struct sembuf));
    // Flagi sa wyzerowane przez calloca
    sem_ops[0].sem_num = ARRAY_STATUS;   // Oczekiwanie na wolna tablice
    sem_ops[0].sem_op = 0;

    sem_ops[1].sem_num = ARRAY_STATUS;  // Zajmujemy tablice, nie pozwalamy innym procesom na jej modyfikacje 
    sem_ops[1].sem_op = 1;       

    sem_ops[2].sem_num = SEND_IDX;      // "Zajmujemy sie" kolejna paczka do wyslania
    sem_ops[2].sem_op = 1;

    sem_ops[3].sem_num = SEND_COUNT;    // Odznaczamy, ze jest jedna paczka mniej do wyslania
    sem_ops[3].sem_op = -1;


    if ( semop(semaphores, sem_ops, 4) == -1 )
        error_exit("Sender: Nie udalo sie wyslac inicjalinych operacji do semafora!", 3);

    orders_struct* orders = shmat(shared_mem, NULL, 0);

    if (orders == (void*) -1)
        error_exit("Sender: Nie udalo sie uzyskac tablicy z pamieci wspoldzielonej!", 4);

    int idx = semctl(semaphores, SEND_IDX, GETVAL, NULL);
    if (idx == -1)  error_exit("Sender: Nie udalo sie odczytac z semaforow indeksu kolejnej paczki do wyslania!", 3);
    if (idx == ORDER_ARRAY_SIZE){
        union semun arg;
        arg.val = 0;
        if (semctl(semaphores, SEND_IDX, SETVAL, arg) == -1)
            error_exit("Sender: Nie udalo sie zresetowac semafora dla indeksu paczki do wyslania!",4);
    }
    idx--;
    orders->orders[idx] *= 3;       // "Wyslamy" paczke

    int orders_to_wrap = semctl(semaphores, WRAP_COUNT, GETVAL, NULL);  // Odczytanie ile trzeba zapakowac (dodajemy nowa paczke)
    int orders_to_send = semctl(semaphores, SEND_COUNT, GETVAL, NULL);  // Odczytanie ilosc paczek do wyslania
    if (orders_to_wrap == -1 || orders_to_send == -1)
        error_exit("Sender: Nie udalo sie odczytac ilosci paczek do zapakowania/do wyslania!",3);   

    //Log
    printf("(%d %ld) Wyslalem zamowienie o wielkosci: %d. Liczba zamównień do przygotowania: %d. Liczba zamównień do wysłania: %d.\n", getpid(), time(NULL), orders->orders[idx], orders_to_wrap, orders_to_send);// formatowanie czasu?

    if (shmdt(orders) == -1)
        error_exit("Sender: Nie udalo sie odlaczyc pamieci wspoldzielonej!",4);

    struct sembuf* closeup = (struct sembuf*) calloc(1, sizeof(struct sembuf));

    closeup[0].sem_num = ARRAY_STATUS;   // Zwolnienie tablicy na wolna tablice
    closeup[0].sem_op = -1;

    if ( semop(semaphores, closeup, 1) == -1 )
        error_exit("Sender: Nie udalo sie wyslac ostatnich operacji do semafora!", 3);
}


void sigusr1_handle(int sig){
    int semaphores = get_semaphores();
    int shared_mem = get_shared_mem();

    while(1){
        usleep(rand_sleep());   // Spi chwile po czym 'wysyla' nowe zamowienie
        int packages_to_send = semctl(semaphores, SEND_COUNT, GETVAL, NULL);
        if (packages_to_send < -1) {
            fprintf(stderr,"Sender: Nie udalo sie odczytac semaforow (przed transakcja)!\n");
        }
        
        if(packages_to_send > 0)
            send_order(semaphores, shared_mem);
    }
}

int main(){
    srand(time(NULL));
    if (signal(SIGUSR1, sigusr1_handle) == SIG_ERR)
        error_exit("Sender: Nie udalo sie podmienic obslugi sygnalu SIGUSR1 (sygnal startowy)!",5);
    while(1){       // Senderzy czekaja na sygnal startowy (SIGUSR1)
        sleep(1);
    }

    return 0;
}