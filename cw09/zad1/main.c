#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <ctype.h>
#include <pthread.h>
#include <math.h>

#define MAX_CLIENT_WANDER_TIME 6
#define MAX_BARBING_TIME 8
#define CLIENT_SPAWNER_TIME 3

int CLIENT_COUNT = -1;
int CHAIR_COUNT = -1;

int barbed_clients_counter = 0;
int is_barber_sleeping = 0;
int waiting_clients_count = 0;
int free_chair = 0;
int next_client = 0;

pthread_t client_on_chair;
pthread_t* clients_waiting_on_chairs = NULL;   
pthread_t* clients_threads = NULL;      //Globalne aby w razie bledu dalo sie usunac

pthread_mutex_t salon_mutex = PTHREAD_MUTEX_INITIALIZER;    // Inicjalizacja przy pomocy makr
pthread_cond_t salon_cond = PTHREAD_COND_INITIALIZER;



// Funkcje pomocnicze
////////////////////////////////////////////
void errExit(const char* err_msg, const char* additional_msg, int exit_code){
    fprintf(stderr, "%s\n", err_msg);
    if (additional_msg != NULL)
        printf("%s\n", additional_msg);
    exit(exit_code);
}

int isANumber(char* number){
    for(int i=0; i < strlen(number); i++)
        if ( !isdigit(number[i]) )
            return 0;
    return 1;
}

void deallocAll(){
    if (clients_threads != NULL)
        free(clients_threads);
    if(clients_waiting_on_chairs != NULL)
        free(clients_waiting_on_chairs);
    pthread_mutex_destroy(&salon_mutex);
    pthread_cond_destroy(&salon_cond);    
}

void initializeGlobalArrays(){
    if(CHAIR_COUNT  <= 0 || CLIENT_COUNT <= 0)
        errExit("Main: Nie odczytano poprawnych wartosci wejsciowych by zainicjalizowac dane!", NULL, 9);
    clients_threads = (pthread_t*)calloc(CLIENT_COUNT, sizeof(pthread_t));
    clients_waiting_on_chairs = (pthread_t*)calloc(CHAIR_COUNT, sizeof(pthread_t));
}
//////////////////////////////////

// Golibroda (barber)
/////////////////////////////////
void nextClientOntoBarbingChair(){
    waiting_clients_count--;
    client_on_chair = clients_waiting_on_chairs[next_client];
    next_client = (next_client + 1) % CHAIR_COUNT;
}

void shaveNextClient(){
    printf("Golibroda: czeka %d klientow, gole klienta %ld\n", waiting_clients_count, client_on_chair);
    pthread_mutex_unlock(&salon_mutex);         // Nie chcemy zajomowac mutex podczas gdy golimy
    
    sleep(rand() % MAX_BARBING_TIME + 1);       // +1 aby zajmowalo to co najmniej jedna sekudne

    pthread_mutex_lock(&salon_mutex);
    barbed_clients_counter++;
    pthread_cancel(client_on_chair);            // Wydaje sie to byc dobrym 'zamknieciem' watku (nie potrzebna nam wartosc zwaracana przez klienta)
}

void barberSweetDreams(){
    printf("Golibroda: ja mam spankoo\n");
    is_barber_sleeping = 1;
    pthread_cond_wait(&salon_cond, &salon_mutex);
    is_barber_sleeping = 0;
}

void * barberRoutine(void* arg){
    while(barbed_clients_counter != CLIENT_COUNT){      // Goli poki nie ogoli wszystkich klientow
        pthread_mutex_lock(&salon_mutex);

        if(waiting_clients_count <= 0)
            barberSweetDreams();
        else
            nextClientOntoBarbingChair();

        shaveNextClient();
        pthread_mutex_unlock(&salon_mutex);
    }
    return (void*) 0;
}
////////////////////////////////

// Klienci
//////////////////////////////
int anyEmptyChair(){
    return waiting_clients_count < CHAIR_COUNT;     // Jesli wszystkie sa zajete to waiting_clients_count == CHAIR_COUNT
}

void wakeUpBarberWeGotACityToBurn(){
    printf("Klient %ld: Wstawaj samuraju, mamy brode do ogolenia\n",pthread_self());
    client_on_chair = pthread_self();
    pthread_cond_broadcast(&salon_cond);          // Budzimy golibrode
    pthread_mutex_unlock(&salon_mutex);
}

void wanderOutsideSalon(){
    while(!anyEmptyChair()){
        pthread_mutex_unlock(&salon_mutex);
        printf("Klient %ld: Zajete\n", pthread_self());
        sleep(rand() % MAX_CLIENT_WANDER_TIME + 1);
        pthread_mutex_lock(&salon_mutex);
    }
}

void waitOnTheChair(){
    clients_waiting_on_chairs[free_chair] = pthread_self();
    free_chair = (free_chair + 1) % CHAIR_COUNT;
    waiting_clients_count++;
    printf("Klient %ld: Poczekalnia, wolne miejsca: %d\n", pthread_self(), CHAIR_COUNT - waiting_clients_count);
    pthread_mutex_unlock(&salon_mutex);
    pause();        // Czeka az zostanie ogolony
}

void * clientRoutine(void* arg){
    pthread_mutex_lock(&salon_mutex);       //unlockowanie mutex znajduje sie w kazdej z funkcji

    if(!anyEmptyChair())
        wanderOutsideSalon();
    if (is_barber_sleeping)
        wakeUpBarberWeGotACityToBurn();
    else
        waitOnTheChair();
    return (void*) 0;
}
///////////////////////


int main(int argc, char** argv){
    srand(time(NULL));
    if (atexit(deallocAll) != 0){
        pthread_mutex_destroy(&salon_mutex);
        pthread_cond_destroy(&salon_cond); 
        errExit("Nie udalo sie zarejestrowac poprawnej funkcji do wywolania na wyjsci!", NULL, 1);
    }

    char needed_args[] = "Potrzebne argumenty: liczba_krzesel liczba_klientow";

    if (argc != 3)
        errExit("Niepoprawna liczba argumentow!", needed_args, 2);

    if ( !isANumber(argv[1]) || atoi(argv[1]) <= 0)
        errExit("Liczba krzesel w salonie musi byc dodatnia liczba!", needed_args, 2);
    if ( !isANumber(argv[2]) || atoi(argv[2]) <= 0)
        errExit("Liczba klientow musi byc dodatnia liczba!", needed_args, 2);
    
    CHAIR_COUNT = atoi(argv[1]);
    CLIENT_COUNT = atoi(argv[2]);
    initializeGlobalArrays();

    clients_threads = (pthread_t*) calloc(CLIENT_COUNT, sizeof(pthread_t));
    pthread_t barber_thread;

    if( pthread_create(&barber_thread, NULL, barberRoutine, NULL) != 0)     // Tworzenie watku golibrody
        errExit("Main: Nie udalo sie utworzyc watku golibrody!", NULL, 3);

    sleep(1);       // Zeby golibroad zasnal

    for(int i=0; i < CLIENT_COUNT; i++){            // Tworzenie watkow klientow
        if(pthread_create(&clients_threads[i], NULL, clientRoutine, NULL) != 0)
            errExit("Main: Nie udalo sie utworzyc watku klienta!", NULL, 3);
        sleep(rand() % CLIENT_SPAWNER_TIME + 1);
    }


    for(int i=0; i < CLIENT_COUNT; i++)             // Tak naprawde oczekiwanie zakonczenie wszystkich watko
        if(pthread_join(clients_threads[i], NULL) != 0)
            errExit("Main: Joinowanie watkow ktoregos klienta sie nie powiodlo!", NULL, 4);

    if(pthread_join(barber_thread, NULL) != 0)
        errExit("Main: Joinowanie watka golibrody sie nie powiodlo!", NULL, 4);

    
    return 0;
}