
#include <unistd.h>
#include <time.h>

#include "utils.h"

sem_t* semaphores[SEMAPHORE_COUNT] = {NULL, NULL, NULL, NULL, NULL, NULL};
int shm_fd = -1;
orders_struct* orders = NULL;


void exit_cleanup(){
    for(int i=0; i < SEMAPHORE_COUNT; i++)         // Zamykamy semafory
        if (semaphores[i] != NULL && sem_close(semaphores[i]) == -1)
            fprintf(stderr,"Receiver: Matko kochana, juz nawet nie ma co zbierac z tych semaforow...\n");

    if (orders != NULL && orders != (void*) -1 && munmap(orders, sizeof(orders_struct)) == -1 )         // Na wypadek zakonczenia programu gdy ktorys z pracownikow mial odwarta pamiec wspoldzielona
        fprintf(stderr,"Receiver: Matko kochana, juz nawet nie ma co zbierac z pamieci wspoldzielonej...\n");
    
}

void error_exit(const char* msg, int code){
    fprintf(stderr,"%s\n",msg);
    exit(code);
}

void new_order(){
    
    if ( sem_wait(semaphores[ARRAY_STATUS]) == -1 )     // Oczekiwanie na wolna tablice      
        error_exit("Receiver: Nie powiodlo sie czekanie semafora ARRAY_STATUS!", 5);

    if ( sem_post(semaphores[FREE_IDX]) == -1)          // Zwiekszamy indeks wolnego miejsca o jeden (kolejne miejsce)
        error_exit("Receiver: Nie udalo sie zwiekszy semafora FREE_IDX!", 6);
    

    orders = mmap(NULL, sizeof(orders_struct), PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (orders == (void*) -1)
        error_exit("Receiver: Nie udalo sie uzyskac tablicy z pamieci wspoldzielonej!", 4);

    int idx;
    if (sem_getvalue(semaphores[FREE_IDX], &idx) == -1)  error_exit("Receiver: Nie udalo sie odczytac wolnego indeksu z semaforow!", 3);
    idx--;
    idx %= ORDER_ARRAY_SIZE;
    orders->orders[idx] = rand_package();

    
    int orders_to_wrap, orders_to_send;
        // Odczytanie ile trzeba zapakowac                                     Odczytanie ilosc paczek do wyslania
    if (sem_getvalue(semaphores[WRAP_COUNT], &orders_to_wrap) == -1 || sem_getvalue(semaphores[SEND_COUNT], &orders_to_send) == -1)  
        error_exit("Receiver: Nie udalo sie odczytac ilosci paczek do zapakowania/do wyslania z semaforow!",3); 
    orders_to_wrap++;       // Dodajemy nowa paczke

    //Log
    struct timeval time_info;
    if(gettimeofday(&time_info, NULL) == -1)            // Stosunkowo maly problem, jesli nie uzyskamy godziny wiec nie konczymy pracy
        fprintf(stderr,"Receiver: Nie udalo sie uzyskac godziny (nie koncze programu)\n");        
    
    char time_str[32];
    strftime(time_str, 32, "%H:%M:%S", localtime(&time_info.tv_sec));
    printf("(%d %s.%ld) Dodalem liczbe: %d. Liczba zamównień do przygotowania: %d. Liczba zamównień do wysłania: %d.\n", getpid(), time_str, time_info.tv_usec / 1000, orders->orders[idx], orders_to_wrap, orders_to_send);

    if (munmap(orders, sizeof(orders_struct)) == -1)
        error_exit("Receiver: Nie udalo sie odlaczyc pamieci wspoldzielonej!",4);

    if (sem_post(semaphores[ARRAY_STATUS]) == -1 )  // Zwolnienie tablicy na wolna tablice
        error_exit("Receiver: Nie udalo sie zwiekszyc semafora ARRAY_STATUS (zwolnic tablicy)!", 14);
    if (sem_post(semaphores[WRAP_COUNT]) == -1)     // Dodanie nowej paczki do zapakowania
        error_exit("Receiver: Nie udalo sie zwiekszyc semafora WRAP_COUNT!",15);
}

int main(){
    srand(time(NULL));
    if ( atexit(exit_cleanup) != 0 ){
        fprintf(stderr,"Receiver: Nie udalo sie ustawic funkcji zamykajacej (atexit())!\n");
        exit(11);
    }
    get_semaphores(semaphores);
    get_shared_mem(&shm_fd);
    
    while(1){
        usleep(rand_sleep());   // Spi chwile po czym 'otrzymuje' nowe zamowienie
        
        int packages_to_send, packages_to_wrap;
        if ( sem_getvalue(semaphores[WRAP_COUNT], &packages_to_wrap) == -1 || sem_getvalue(semaphores[SEND_COUNT], &packages_to_send) == -1)
            error_exit("Receiver: Nie udalo sie odczytac semaforow (przed transakcja)!",16);
        
        if(packages_to_send + packages_to_wrap < ORDER_ARRAY_SIZE)
            new_order();
    }

    return 0;
}