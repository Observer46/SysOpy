#ifndef _UTILS_H
#define _UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>

#include "systemV_shop_settings.h"

// Ustawienia semaforow (wykorzystawane do tych celow ze wzglednu na atomowosc swoich operacji)
#define ARRAY_STATUS 0  // Status tablicy (modyfikowana przez kogos badz tez wolna)
#define FREE_IDX 1      // Pierwsze wolne miejsce, pod ktore mozemy dokladac zamowienia
#define WRAP_IDX 2      // Indeks paczki, ktora nalezy zapakowac jako kolejna
#define WRAP_COUNT 3    // Ilosc paczek do zapakowania (parametr m)
#define SEND_IDX 4      // Indeks paczki, ktora nalezy wslac jako kolejna
#define SEND_COUNT 5    // Ilosc paczek do wyslania

typedef struct orders_struct{
    //int to_prepare;
    //int to_send;
    int orders[ORDER_ARRAY_SIZE];
}orders_struct;

// Tak, bylo tutaj ctrl+c, ctrl+v z linuxowego mana :)
typedef union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
}semun;


int rand_package();
int rand_sleep();
int get_semaphores();
int get_shared_mem();

#endif