#include "utils.h"

int rand_package(){
    return rand() % (RAND_MAX_VAL - RAND_MIN_VAL) + RAND_MIN_VAL;
}

int rand_sleep(){
    return rand() % (SLEEP_MAX - SLEEP_MIN) + SLEEP_MIN;
}

int get_semaphores(){
    key_t sem_key = ftok(getenv("HOME"), 'a');
    int semaphores = semget(sem_key, 0, 0);
    return semaphores;
}

int get_shared_mem(){
    key_t shm_key = ftok(getenv("HOME"), 'b');
    int shared_mem = shmget(shm_key, 0, 0);
    return shared_mem;
}