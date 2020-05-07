
#include <unistd.h>
#include <time.h>

#include "utils.h"

void error_exit(const char* msg, int code){
    fprintf(stderr,"%s\n",msg);
    exit(code);
}

void new_order(int semaphores, int shared_mem){
    struct sembuf* sem_ops = (struct sembuf*)calloc(3, sizeof(struct sembuf));

    // Flagi sa wyzerowane przez calloca
    sem_ops[0].sem_num = ARRAY_STATUS;   // Oczekiwanie na wolna tablice
    sem_ops[0].sem_op = 0;

    sem_ops[1].sem_num = ARRAY_STATUS;  // Zajmujemy tablice, nie pozwalamy innym procesom na jej modyfikacje 
    sem_ops[1].sem_op = 1;       

    sem_ops[2].sem_num = FREE_IDX;      // Zwiekszamy indeks wolnego miejsca o jeden (kolejne miejsce)
    sem_ops[2].sem_op = 1;


    if ( semop(semaphores, sem_ops, 3) == -1 )
        error_exit("Receiver: Nie udalo sie wyslac inicjalinych operacji do semafora!", 3);

    orders_struct* orders = shmat(shared_mem, NULL, 0);

    if (orders == (void*) -1)
        error_exit("Receiver: Nie udalo sie uzyskac tablicy z pamieci wspoldzielonej!", 4);

    int idx = semctl(semaphores, FREE_IDX, GETVAL, NULL);
    if (idx == -1)  error_exit("Receiver: Nie udalo sie odczytac wolnego indeksu z semaforow!", 3);
    if (idx == ORDER_ARRAY_SIZE){
        union semun arg;
        arg.val = 0;
        if (semctl(semaphores, FREE_IDX, SETVAL, arg) == -1)       
            error_exit("Receiver: Nie udalo sie zresetowac semafora dla wolnego indeksu!",4);
    }
    idx--;
    orders->orders[idx] = rand_package();

    int orders_to_wrap = 1 + semctl(semaphores, WRAP_COUNT, GETVAL, NULL);  // Odczytanie ile trzeba zapakowac (+1 bo dodajemy nowa paczke)
    int orders_to_send = semctl(semaphores, SEND_COUNT, GETVAL, NULL);      // Odczytanie ilosc paczek do wyslania
    if (orders_to_wrap == -1 || orders_to_send == -1)
        error_exit("Receiver: Nie udalo sie odczytac ilosci paczek do zapakowania/do wyslania!",3);   

    //Log
    struct timeval time_info;
    if(gettimeofday(&time_info, NULL) == -1)            // Stosunkowo maly problem, jesli nie uzyskamy godziny wiec nie konczymy pracy
        fprintf(stderr,"Receiver: Nie udalo sie uzyskac godziny (nie koncze programu)\n");        
    
    char time_str[32];
    strftime(time_str, 32, "%H:%M:%S", localtime(&time_info.tv_sec));
    printf("(%d %s.%ld) Dodałem liczbę: %d. Liczba zamównień do przygotowania: %d. Liczba zamównień do wysłania: %d.\n", getpid(), time_str, time_info.tv_usec / 1000, orders->orders[idx], orders_to_wrap, orders_to_send);

    if (shmdt(orders) == -1)
        error_exit("Receiver: Nie udalo sie odlaczyc pamieci wspoldzielonej!",4);

    struct sembuf* closeup = (struct sembuf*) calloc(2, sizeof(struct sembuf));

    closeup[0].sem_num = ARRAY_STATUS;  // Zwolnienie tablicy
    closeup[0].sem_op = -1;

    closeup[1].sem_num = WRAP_COUNT;    // Dodanie nowej paczki do zapakowania
    closeup[1].sem_op = 1;

    if ( semop(semaphores, closeup, 2) == -1 )
        error_exit("Receiver: Nie udalo sie wyslac ostatnich operacji do semafora!", 3);
}

int main(){
    srand(time(NULL));
    int semaphores = get_semaphores();
    int shared_mem = get_shared_mem();

    while(1){
        usleep(rand_sleep());   // Spi chwile po czym 'otrzymuje' nowe zamowienie
        int packages_to_wrap = semctl(semaphores, WRAP_COUNT, GETVAL, NULL);
        int packages_to_send = semctl(semaphores, SEND_COUNT, GETVAL, NULL);
        if (packages_to_send < -1 || packages_to_send < -1)
            error_exit("Receiver: Nie udalo sie odczytac semaforow (przed transakcja)!\n", 13);
        
        if(packages_to_send + packages_to_wrap < ORDER_ARRAY_SIZE)
            new_order(semaphores, shared_mem);
    }

    return 0;
}