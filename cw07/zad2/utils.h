#ifndef _UTILS_H
#define _UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <fcntl.h>


#include "posix_shop_settings.h"

// Ustawienia semaforow (wykorzystawane do tych celow ze wzglednu na atomowosc swoich operacji)
#define ARRAY_STATUS 0  // Status tablicy (modyfikowana przez kogos badz tez wolna)
#define FREE_IDX 1      // Pierwsze wolne miejsce, pod ktore mozemy dokladac zamowienia
#define WRAP_IDX 2      // Indeks paczki, ktora nalezy zapakowac jako kolejna
#define WRAP_COUNT 3    // Ilosc paczek do zapakowania (parametr m)
#define SEND_IDX 4      // Indeks paczki, ktora nalezy wslac jako kolejna
#define SEND_COUNT 5    // Ilosc paczek do wyslania

const char* semaphores_names[SEMAPHORE_COUNT];

typedef struct orders_struct{
    int orders[ORDER_ARRAY_SIZE];
}orders_struct;

int rand_package();
int rand_sleep();
void get_semaphores(sem_t** semaphores);
void get_shared_mem(int* shm_fd);

#endif