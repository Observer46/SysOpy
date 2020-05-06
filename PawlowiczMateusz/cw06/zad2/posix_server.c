#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <mqueue.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>

#include "posix_chat_settings.h"

int my_id = 0;
mqd_t server_queue_id;

int id_of_client[MAX_CLIENTS];
mqd_t client_queues_id[MAX_CLIENTS];
char client_queue_names[MAX_CLIENTS][QUEUE_NAME_LENGTH];
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
    fprintf(stderr,"Wystapil problem z usuwaniem kolejki %s\n", SERVER_NAME);
    exit(2);
}

void err_exit_close_queue(mqd_t queue_id){
    fprintf(stderr,"Wystapil problem z zamknieciem kolejki %d po stronie serwera!\n", queue_id);
    exit(3);
}

void err_exit_mq_open(const char* queue_name){
    fprintf(stderr,"Nie udalo sie uzyskac kolejki %s przy pomocy funkcji mq_open()!\n", queue_name);
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
    if (mq_close(client_queues_id[idx]) == -1)
        err_exit_close_queue(client_queues_id[idx]);
    strcpy(client_queue_names[idx], "");
    id_of_client[idx] = chat_status[idx] = client_queues_id[idx] = -1;
    client_count--;
}

void server_stop(){         // Podawane do atexit()
    printf("\nZamykanie serwera...\n");
    char msg[MAX_MSG_LENGTH];

    for (int i=0; i < MAX_CLIENTS; i++){
        if (id_of_client[i] != -1){
            mqd_t client_queue = client_queues_id[i];
            if (mq_send(client_queue, msg, MAX_MSG_LENGTH, STOP) == -1)   // Wysylamy do klienta info o zamkniecu serwera
                err_exit_msg("mq_send()","STOP");

            char reply[MAX_MSG_LENGTH];
            unsigned prio;
            if (mq_receive(server_queue_id, reply, MAX_MSG_LENGTH, &prio) == -1)   // Czekamy na 'potwierdzenie' od klienta
                err_exit_msg("mq_receive()","STOP");
            if (prio != STOP)
                err_exit_msg("mq_receive()","STOP - zly komunikat");

            if (mq_close(client_queue) == -1)   // Zamykamy kolejke klienta
                err_exit_close_queue(client_queue);
            // Z sama wiadomoscia nic nie musimy robic, po prostu chcemy uzyskac info o tym, czy klient sie rozlaczyl juz
        }
    }
    if (mq_close(server_queue_id) == -1)
        err_exit_close_queue(server_queue_id);

    if (mq_unlink(SERVER_NAME) == -1)      // Usuwamy kolejke serwera
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
    
    mqd_t other_queue = client_queues_id[other_idx];
    char msg[MAX_MSG_LENGTH];
    
    if(mq_send(other_queue, msg, MAX_MSG_LENGTH, DISCONNECT) == -1)
        err_exit_msg("mq_send()","DISCONNECT");
    chat_status[other_idx] = chat_status[idx] = -2;     // Oznaczamy, ze przestali rozmawiac
}

void initialize_server(){
    srand(time(NULL));
    struct mq_attr attributes;
    attributes.mq_maxmsg = 8;
    attributes.mq_msgsize = 2048;

    server_queue_id = mq_open(SERVER_NAME, O_RDONLY|O_CREAT,0666,&attributes);
    if (server_queue_id == -1){
        if (errno == EAGAIN)
            printf("EAGAIN\n");
        if(errno == EBADF)
            printf("EBADF\n");
        if(errno == EINTR)
            printf("EINTR\n");
        if (errno == EINVAL)
            printf("EINVAL\n");
        if(errno == EMSGSIZE)
            printf("EMSGSIZE\n");
        if (errno == ETIMEDOUT)
            printf("ETIMEDOUT\n");
        err_exit_mq_open(SERVER_NAME);
    }

    if (atexit(server_stop) != 0){         // Obsluga wyjsca
        fprintf(stderr, "Nie udalo sie ustawic poprawnej funkcji zamykajcej program klienta!\n");
        exit(42);
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

    char msg_to_1[MAX_MSG_LENGTH];
    strcpy(msg_to_1, "");
    msg_to_1[0] = (char) client2;
    strcat(msg_to_1, client_queue_names[idx2]);

    char msg_to_2[MAX_MSG_LENGTH];
    strcpy(msg_to_2, "");
    msg_to_2[0] = (char) client1;
    strcat(msg_to_2, client_queue_names[idx1]);
    
    if(mq_send(client_queues_id[idx1], msg_to_1, MAX_MSG_LENGTH, CONNECT) == -1)
        err_exit_msg("mq_send()","CONNECT");

    if (failed)
        return;
    if(mq_send(client_queues_id[idx2], msg_to_2, MAX_MSG_LENGTH, CONNECT) == -1)
        err_exit_msg("mq_send()","CONNECT");
   

    chat_status[idx1] = client2;      // Zaznaczamy, ze sa razem w czacie
    chat_status[idx2] = client1; 
    
}

void send_list(int client_id){
    char msg[MAX_MSG_LENGTH];
    strcpy(msg,"");
    int buf_len = 0;
    for(int i=0; i < MAX_CLIENTS; i++){
        if (id_of_client[i] != -1){
            buf_len += sprintf(msg + buf_len, "Klient %d: ", id_of_client[i]);
            if (chat_status[i] == -2)
                buf_len += sprintf(msg + buf_len,"dostepny\n");
            else
                buf_len += sprintf(msg + buf_len,"czatuje z klientem %d\n", chat_status[i]); 
        }   
    }
    int adr_idx = client_idx(client_id);
    if (adr_idx == -1)
        err_exit_client(client_id);
    mqd_t adressee_queue = client_queues_id[adr_idx];

    if (mq_send(adressee_queue, msg, MAX_MSG_LENGTH, LIST) == -1)
        err_exit_msg("mq_send()","LIST");
}

void init_client(mqd_t queue_id){
    char reply[MAX_MSG_LENGTH];
    int new_client_id = (client_count >= MAX_CLIENTS) ? 0 : client_id_counter;
    int failed = 0;
    if (new_client_id == -1){
        printf("Serwer: Osiagnieto limit podlaczonych klientow - nie mozna podlaczyc kolejnego!\n");
        failed = 1;
    }
    
    if (mq_send(queue_id, reply, MAX_MSG_LENGTH, INIT) == -1)
        err_exit_msg("mq_send()","INIT");
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
    char msg[MAX_MSG_LENGTH];
    unsigned prio;
    printf("%d\n",server_queue_id);
    if  (mq_receive(server_queue_id, msg, MAX_MSG_LENGTH, &prio) == -1){      // Znajduje wedlug najstarsza wiadomosc wg priorytetu
        if (errno == EAGAIN)
            printf("EAGAIN\n");
        if(errno == EBADF)
            printf("EBADF\n");
        if(errno == EINTR)
            printf("EINTR\n");
        if (errno == EINVAL)
            printf("EINVAL\n");
        if(errno == EMSGSIZE)
            printf("EMSGSIZE\n");
        if (errno == ETIMEDOUT)
            printf("ETIMEDOUT\n");
        err_exit_msg("mq_receive()","FATAL ERROR");
    }

    int client_id = (int) msg[0];
    if (prio == STOP){
        client_cleanup(client_id);
    }
    else if (prio == DISCONNECT){
        int client_to_dc = client_id;
        end_chat(client_to_dc);
    }
    else if (prio == LIST){  
        send_list(client_id); 
    }
    else if (prio == CONNECT){     // msg[0] - id drugiego klienta, po nim nazwa kolejki pierwszego klienta
        int initiator_id;
        int client_id = (int) msg[0];
        char initiator_queue_name[QUEUE_NAME_LENGTH];
        strcpy(initiator_queue_name, msg + 1);      // Bez pierwszego znaku
        for (int i=0; i < MAX_CLIENTS; i++)
            if (strcmp(initiator_queue_name, client_queue_names[i]) == 0)
                initiator_id = id_of_client[i];
        
        connect_clients(initiator_id, client_id);
    }
    else if (prio == INIT){   
        char new_queue_name[QUEUE_NAME_LENGTH];
        strcpy(new_queue_name, msg);
        //new_queue_name[strlen(new_queue_name)] = '\0';
        mqd_t new_queue_id = mq_open(new_queue_name, O_RDONLY);
        if (errno == EACCES)
            printf("EACCES\n");
        if(errno == EEXIST)
            printf("EEXIST\n");
        if(errno == EINVAL)
            printf("EINVAL\n");
        if (errno == ENAMETOOLONG)
            printf("ENAMETOOLONG\n");
        if(errno == ENOSPC)
            printf("ENOSPC\n");
        if (errno == ENOMEM)
            printf("ENOMEM\n");
        if (errno == ENOENT)
            printf ("ENOENT\n");
        printf("%d\n",errno); 
        if (new_queue_id == -1)
            err_exit_mq_open(new_queue_name);

        init_client(new_queue_id);
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