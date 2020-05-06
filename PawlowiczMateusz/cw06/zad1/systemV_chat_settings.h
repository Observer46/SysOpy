#ifndef _CHAT_SETTINGS_H
#define _CHAT_SETTINGS_H


#define MAX_MSG_LENGTH 2048        // Potegi dwojek sa fajne
#define MAX_CLIENTS 16

typedef enum msg_type{
    STOP =1,
    DISCONNECT = 2,
    LIST = 3,
    CONNECT = 4,
    INIT = 5
} msg_type;

typedef struct my_msg{
    long msg_type;
    char text[MAX_MSG_LENGTH];
    unsigned client_id;
    int queue_id;       // Klucz kolejki jako identyfikator kolejki
} my_msg;

const int MSG_SIZE = sizeof(my_msg) - sizeof(long);
const int SERVER_PROJECT_ID = 1;    // Klienci nie moga go wylosowac

#endif