#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <time.h>
#include <ctype.h>

#include "systemV_chat_settings.h"

int my_id = -1;
int server_queue_id;
key_t key_for_server_queue;

int id_of_client[MAX_CLIENTS];
int client_queues_id[MAX_CLIENTS];
int chat_status[MAX_CLIENTS];
int client_id_counter = 1;
int client_count = 0;

int isANumber(char* number){
    for(int i=0; i < strlen(number); i++)
        if ( !isdigit(number[i]) )
            return 0;
    return 1;
}

// Bledy
void err_exit_msg(char* function_name, char* msg_type){
    fprintf(stderr,"Serwer: Wystapil problem z obsluga komunikatu %s w funkcji %s!\n", msg_type, function_name);
    exit(1);
}

void err_exit_del_queue(){
    fprintf(stderr,"Wystapil problem z usuwaniem kolejki %d u klienta %d", server_queue_id, my_id);
    exit(2);
}

void err_exit_ftok(){
    fprintf(stderr, "Nie udalo sie uzyskac klucza przy pomocy funkcji ftok()!\n");
    exit(3);
}

void err_exit_msgget(){
    fprintf(stderr,"Nie udalo sie uzyskac kolejki przy pomocy funkcji msgget()!\n");
    exit(4);
}

void err_exit_client(int client_id){
    fprintf(stderr,"Nie ma klienta %d!\n", client_id);
    exit(5);
}

void err_exit_chat(int client_id){
    fprintf(stderr,"Klient %d z nikim nie rozmawia!\n", client_id);
    exit(6);
}
///////////////////////////////////////

int client_idx(int client_id){
    for(int i=0; i < MAX_CLIENTS; i++)
        if (id_of_client[i] == client_id)
            return i;
    return -1;
}

// Obsluga komunikatow
void client_cleanup(int client_id){
    int idx = client_idx(client_id);
    if (idx == -1)
        err_exit_client(client_id);
    id_of_client[idx] = chat_status[idx] = client_queues_id[idx] = -1;
    client_count--;
}

void server_stop(){         // Podawane do atexit()
    printf("\nZamykanie serwera...\n");
    my_msg msg;
    msg.client_id = my_id;
    msg.queue_id = server_queue_id;
    msg.msg_type = STOP;

    for (int i=0; i < MAX_CLIENTS; i++){
        if (id_of_client[i] != -1){
            int client_queue = client_queues_id[i];
            if (msgsnd(client_queue, &msg, MSG_SIZE, 0) == -1)   // Wysylamy do klienta info o zamkniecu serwera
                err_exit_msg("msgsnd()","STOP");

            my_msg reply_from_client;
            if (msgrcv(server_queue_id, &reply_from_client, MSG_SIZE, STOP, 0) == -1)   // Czekamy na 'potwierdzenie' od klienta
                err_exit_msg("msgrcv()","STOP");
            // Z sama wiadomoscia nic nie musimy robic, po prostu chcemy uzyskac info o tym, czy klient sie rozlaczyl juz
        }
    }

    if (msgctl(server_queue_id, IPC_RMID, NULL) == -1)      // Usuwamy kolejke serwera
        err_exit_del_queue();
    printf("\nSerwer zakonczyl zadanie - kolejka usunieta!\n");
    exit(0);
}

void end_chat(int client_to_dc){
    int idx = client_idx(client_to_dc);
    if (idx == -1)
        err_exit_client(client_to_dc);
    if (chat_status[idx] == -1)
        err_exit_chat(client_to_dc);
    
    int other_id = chat_status[idx];
    int other_idx = client_idx(other_id);
    if (other_id < 0 && other_idx < 0)
        err_exit_chat(other_id);
    
    int other_queue = client_queues_id[other_idx];
    my_msg msg;
    msg.msg_type = DISCONNECT;
    
    if(msgsnd(other_queue, &msg, MSG_SIZE, 0) == -1)
        err_exit_msg("msgsnd()","DISCONNECT");
    chat_status[other_idx] = chat_status[idx] = -2;     // Oznaczamy, ze przestali rozmawiac
}

void initialize_server(){
    srand(time(NULL));
    char* home_path = getenv("HOME"); 
    key_for_server_queue = ftok(home_path, SERVER_PROJECT_ID);
    if (key_for_server_queue == -1)
        err_exit_ftok();

    server_queue_id = msgget(key_for_server_queue, IPC_CREAT|0666);
    if (server_queue_id == -1)
        err_exit_msgget();

    if (atexit(server_stop) != 0){         // Obsluga wyjsca
        fprintf(stderr, "Nie udalo sie ustawic poprawnej funkcji zamykajcej program klienta!\n");
        exit(5);
    }

    for(int i=0; i < MAX_CLIENTS; i++)
        id_of_client[i] = client_queues_id[i] = chat_status[i] = -1;

    printf("Poprawnie uruchomiono serwer: %d\n",server_queue_id);
}
///////////////////

void connect_clients(int client1, int client2){
    printf("Laczenie klientow: %d %d\n", client1, client2);
    int failed = 0;
    int idx1 = client_idx(client1); // Na pewno jest
    
    int idx2 = client_idx(client2);
    if (idx2 == -1){
        failed = 1;
        client2 = -1;
    }
    if (client1 == client2){
        failed = 1;
        client2 = client1;
    }
    if (chat_status[idx2] > 0){
        failed = 1; // Wysylamy info, ze drugi uzytkownik jest w czacie z kims innym
        client2 = chat_status[idx2];
    }

    my_msg msg_to_1;
    msg_to_1.msg_type = CONNECT;
    msg_to_1.client_id = client2;
    msg_to_1.queue_id = client_queues_id[idx2];

    my_msg msg_to_2;
    msg_to_2.msg_type = CONNECT;
    msg_to_2.client_id = client1;
    msg_to_2.queue_id = client_queues_id[idx1];
    
    if(msgsnd(client_queues_id[idx1], &msg_to_1, MSG_SIZE, 0) == -1)
        err_exit_msg("msgsnd()","CONNECT");

    if (failed)
        return;
    if(msgsnd(client_queues_id[idx2], &msg_to_2, MSG_SIZE, 0) == -1)
        err_exit_msg("msgsnd()","CONNECT");
   

    chat_status[idx1] = (int) client2;      // Zaznaczamy, ze sa razem w czacie
    chat_status[idx2] = (int) client1; 
    
}

void send_list(int client_id){
    my_msg reply_to_client;
    reply_to_client.msg_type = LIST;
    strcpy(reply_to_client.text,"");
    int buf_len = 0;
    for(int i=0; i < MAX_CLIENTS; i++){
        if (id_of_client[i] != -1){
            buf_len += sprintf(reply_to_client.text + buf_len, "Klient %d: ", id_of_client[i]);
            if (chat_status[i] == -2)
                buf_len += sprintf(reply_to_client.text + buf_len,"dostepny\n");
            else
                buf_len += sprintf(reply_to_client.text + buf_len,"czatuje z klientem %d\n", chat_status[i]); 
        }   
    }
    int adr_idx = client_idx(client_id);
    if (adr_idx == -1)
        err_exit_client(client_id);
    int adressee_queue = client_queues_id[adr_idx];

    if (msgsnd(adressee_queue, &reply_to_client, MSG_SIZE, 0) == -1)
        err_exit_msg("msgsnd()","LIST");
}

void init_client(int queue_id){
    
    my_msg reply;
    reply.msg_type = INIT;
    int new_client_id = (client_count >= MAX_CLIENTS) ? -1 : client_id_counter;
    reply.client_id = new_client_id;
    int failed = 0;
    if (new_client_id == -1){
        printf("Serwer: Osiagnieto limit podlaczonych klientow - nie mozna podlaczyc kolejnego!\n");
        failed = 1;
    }
    
    if (msgsnd(queue_id, &reply, MSG_SIZE, 0) == -1)
        err_exit_msg("msgsnd()","INIT");
    if (failed)
        return;
    
    
    int idx = 0;
    while(idx < MAX_CLIENTS){
        if (id_of_client[idx] == -1)
            break;
        idx++;
    }
    printf("Inicjowanie klienta o kolejce: %d\n",queue_id);
    client_queues_id[idx] = queue_id;
    id_of_client[idx] = new_client_id;
    chat_status[idx] = -2; // Wolny do rozmowy

    client_count++;
    client_id_counter++;
}


void check_messages(){
    my_msg msg;
    if  (msgrcv(server_queue_id, &msg, MSG_SIZE, -6, 0) == -1)      // Znajduje wedlug najstarsza wiadomosc wg priorytetu
        err_exit_msg("msgrcv","FATAL ERROR");


    if (msg.msg_type == STOP){
        client_cleanup(msg.client_id);
    }
    else if (msg.msg_type == DISCONNECT){
        int client_to_dc = msg.client_id;
        end_chat(client_to_dc);
    }
    else if (msg.msg_type == LIST){  
        send_list(msg.client_id); 
    }
    else if (msg.msg_type == CONNECT){     // msg.queue_id - kolejka wysyslajacego, msg.client_id - klient z ktorym chce sie polaczyc
        int initiator_id;
        for (int i=0; i < MAX_CLIENTS; i++)
            if (client_queues_id[i] == msg.queue_id)
                initiator_id = id_of_client[i];
        connect_clients(initiator_id, msg.client_id);
    }
    else if (msg.msg_type == INIT){       
        init_client(msg.queue_id);
    }
    else {
        printf("Serwer: Odebrano nieznany typ wiadomosci!\n");
        printf("Serwer: W sumie to nie wiem co z nim zrobic wiec nie zrobie nic...\n");       
    }

}

void handle_SIGINT(int sig){
    exit(0);
}

int main(){
    signal(SIGINT, handle_SIGINT);
    initialize_server();
    while (1)
        check_messages();
    return 0;
}