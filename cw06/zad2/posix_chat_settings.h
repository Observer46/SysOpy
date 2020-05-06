#ifndef _CHAT_SETTINGS_H
#define _CHAT_SETTINGS_H


#define MAX_MSG_LENGTH 2048        // Potegi dwojek sa fajne
#define MAX_CLIENTS 16
#define QUEUE_NAME_LENGTH 16

typedef enum msg_type{
    STOP = 5,
    DISCONNECT = 4,
    LIST = 3,
    CONNECT = 2,
    INIT = 1
} msg_type;

const char* SERVER_NAME = "/CHAT_SERVER";

#endif