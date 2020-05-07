
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include "utils.h"

sem_t* semaphores[SEMAPHORE_COUNT] = {NULL, NULL, NULL, NULL, NULL, NULL};
int shm_fd = -1;
orders_struct* orders = NULL;

void exit_cleanup(){
    for(int i=0; i < SEMAPHORE_COUNT; i++)         // Zamykamy semafory
        if (semaphores[i] != NULL && sem_close(semaphores[i]) == -1)
            fprintf(stderr,"Wrapper: Matko kochana, juz nawet nie ma co zbierac z tych semaforow...\n");

    if (orders != NULL && orders != (void*) -1 && munmap(orders, sizeof(orders_struct)) == -1 )         // Na wypadek zakonczenia programu gdy ktorys z pracownikow mial odwarta pamiec wspoldzielona
        fprintf(stderr,"Wrapper: Matko kochana, juz nawet nie ma co zbierac z pamieci wspoldzielonej...\n");
    
}

void error_exit(const char* msg, int code){
    fprintf(stderr,"%s\n",msg);
    exit(code);
}

void wrap_order(){
    if ( sem_wait(semaphores[ARRAY_STATUS]) == -1 )     // Oczekiwanie na wolna tablice      
        error_exit("Wrapper: Nie powiodlo sie czekanie semafora ARRAY_STATUS!", 5);

    if ( sem_post(semaphores[WRAP_IDX]) == -1)          // Zwiekszamy indeks kolejnej paczki do zapakowania
        error_exit("Wrapper: Nie udalo sie zwiekszy semafora WRAP_IDX!", 6);

    if ( sem_wait(semaphores[WRAP_COUNT]) == -1)         // Zmniejszamy ilosc paczek do zapakowania
        error_exit("Wrapper: Nie udalo sie zwiekszy semafora WRAP_COUNT!", 7);

    orders = mmap(NULL, sizeof(orders_struct), PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (orders == (void*) -1)
        error_exit("Wrapper: Nie udalo sie uzyskac tablicy z pamieci wspoldzielonej!", 4);

    int idx;
    if (sem_getvalue(semaphores[WRAP_IDX], &idx) == -1)  error_exit("Wrapper: Nie udalo sie odczytac z semaforow indeksu paczki do zapakowania!", 3);
    idx--;
    idx %= ORDER_ARRAY_SIZE;
    orders->orders[idx] *= 2;


    int orders_to_wrap, orders_to_send;
        // Odczytanie ile trzeba zapakowac                                     Odczytanie ilosc paczek do wyslania
    if (sem_getvalue(semaphores[WRAP_COUNT], &orders_to_wrap) == -1 || sem_getvalue(semaphores[SEND_COUNT], &orders_to_send) == -1)  
        error_exit("Wrapper: Nie udalo sie odczytac ilosci paczek do zapakowania/do wyslania z semaforow!",3);

    //Log
    struct timeval time_info;
    if(gettimeofday(&time_info, NULL) == -1)            // Stosunkowo maly problem, jesli nie uzyskamy godziny wiec nie konczymy pracy
        fprintf(stderr,"Wrapper: Nie udalo sie uzyskac godziny (nie koncze programu)\n");        
    
    char time_str[32];
    strftime(time_str, 32, "%H:%M:%S", localtime(&time_info.tv_sec));
    printf("(%d %s.%ld) Przygotowalem zamowienie o wielkosci: %d. Liczba zamównień do przygotowania: %d. Liczba zamównień do wysłania: %d.\n", getpid(), time_str, time_info.tv_usec / 1000, orders->orders[idx], orders_to_wrap, orders_to_send);

    if (munmap(orders, sizeof(orders_struct)) == -1)
        error_exit("Wrapper: Nie udalo sie odlaczyc pamieci wspoldzielonej!",4);

    if (sem_post(semaphores[ARRAY_STATUS]) == -1 )  // Zwolnienie tablicy na wolna tablice
        error_exit("Wrapper: Nie udalo sie zwiekszyc semafora ARRAY_STATUS (zwolnic tablicy)!", 14);
    if (sem_post(semaphores[SEND_COUNT]) == -1)     // Dodanie nowej paczki do kolejki do wyslania
        error_exit("Wrapper: Nie udalo sie zwiekszyc semafora SEND_COUNT!",15);
}

// Sygnal SIGUSR1 uruchamia program (wrapperzy i senderzy czekaja az wszystkie ich podprogramy zostana uruchomione)
void sigusr1_handle(int sig){
    while(1){
        usleep(rand_sleep());   // Spi chwile po czym 'pakuje' kolejne zamowienie
        int packages_to_wrap;
        if (sem_getvalue(semaphores[WRAP_COUNT], &packages_to_wrap) == -1) {
            fprintf(stderr,"Wrapper: Nie udalo sie odczytac semaforow (przed transakcja)!\n");
        }
        
        if(packages_to_wrap > 0)
            wrap_order();
    }
}



int main(){
    srand(time(NULL));
    get_semaphores(semaphores);
    get_shared_mem(&shm_fd);

    if ( atexit(exit_cleanup) != 0 ){
        fprintf(stderr,"Wrapper: Nie udalo sie ustawic funkcji zamykajacej (atexit())!\n");
        exit(11);
    }

    if (signal(SIGUSR1, sigusr1_handle) == SIG_ERR)
        error_exit("Wrapper: Nie udalo sie podmienic obslugi sygnalu SIGUSR1 (sygnal startowy)!",5);

    while(1){       // Wrapperzy czekaja na sygnal startowy (SIGUSR1)
        sleep(1);
    }

    return 0;
}