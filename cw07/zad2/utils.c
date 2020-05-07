#include "utils.h"

const char* semaphores_names[SEMAPHORE_COUNT] = {"/ARRAY_STATUS", "/FREE_IDX", "/WRAP_IDX", "/WRAP_COUNT", "/SEND_IDX", "/SEND_COUNT"};

int rand_package(){
    return rand() % (RAND_MAX_VAL - RAND_MIN_VAL) + RAND_MIN_VAL;
}

int rand_sleep(){
    return (rand() % (SLEEP_MAX - SLEEP_MIN) + SLEEP_MIN) * 10000;  // od 0.5 sekundy do 1.5 sekundy spania (wartosci co 0.01)
}

void get_semaphores(sem_t** semaphores){
    for(int i=0; i < SEMAPHORE_COUNT; i++){
        semaphores[i] = sem_open(semaphores_names[i], O_RDWR);
        if (semaphores[i] == SEM_FAILED){
            fprintf(stderr,"Nie udalo sie otworzyc jednego z semaforow!\n");
            exit(10);
        }
    }

}

void get_shared_mem(int* shm_fd){
    *shm_fd = shm_open(SHARED_MEM, O_RDWR, 0);
    if (*shm_fd == -1){
        fprintf(stderr,"Nie udalo sie otworzyc pamieci wspoldzielonej\n");
        exit(10);
    }
}