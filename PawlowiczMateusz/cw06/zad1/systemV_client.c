#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>

#include "systemV_chat_settings.h"

int my_id;
int my_queue_id;
int server_queue_id;
key_t key_for_my_queue;


void chat_mode();       // Zadeklarowane dla funkcji connect()

int isANumber(char* number){
    for(int i=0; i < strlen(number); i++)
        if ( !isdigit(number[i]) )
            return 0;
    return 1;
}

// Bledy
void err_exit_msg(char* function_name, char* msg_type){
    fprintf(stderr,"Wystapil problem z obsluga komunikatu %s w funkcji %s!\n", msg_type, function_name);
    exit(1);
}

void err_exit_del_queue(){
    fprintf(stderr,"Wystapil problem z usuwaniem kolejki %d u klienta %d", my_queue_id, my_id);
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
///////////////////////////////////////

// Obsluga komunikatow
void stop(){
    my_msg msg;
    msg.client_id = my_id;
    //msg.queue_id = my_queue_id;
    msg.msg_type = STOP;

    if (msgsnd(server_queue_id, &msg, MSG_SIZE, 0) == -1)      // Najpierw wysylamy informacje do serwera
        printf("Nie udao sie wyslac wiadomosci konczacej do serwera!\n");                            

    if (msgctl(my_queue_id, IPC_RMID, NULL) == -1)
        err_exit_del_queue();

    printf("\n**Koniec dzialania klienta**\n");
}

void disconnect(){
    my_msg msg;
    msg.msg_type = DISCONNECT;
    msg.client_id = my_id;
    msg.queue_id = my_queue_id;
    if (msgsnd(server_queue_id, &msg, MSG_SIZE, 0) == -1)
        err_exit_msg("msgsnd","DISCONNECT");
    
}

void list(){
    my_msg msg;
    msg.msg_type = LIST;
    msg.client_id = my_id;
    msg.queue_id = my_queue_id;
    if (msgsnd(server_queue_id, &msg, MSG_SIZE, 0) == -1)
        err_exit_msg("msgsnd","LIST");
    
    my_msg reply_from_server;
    if (msgrcv(my_queue_id, &reply_from_server, MSG_SIZE, LIST, 0) == -1)
        err_exit_msg("msgrcv","LIST");
    printf("\tLista dostepnych klientow:\n%s",reply_from_server.text);
}

void connect(int other_id){
    my_msg msg;
    msg.msg_type = CONNECT;
    msg.client_id = other_id;
    msg.queue_id = my_queue_id;     // Aby latwo okreslic kto wyslal
    if (msgsnd(server_queue_id, &msg, MSG_SIZE, 0) == -1)
        err_exit_msg("msgsnd","CONNECT");
    
    my_msg reply_from_server;
    if (msgrcv(my_queue_id, &reply_from_server, MSG_SIZE, CONNECT, 0) == -1)
        err_exit_msg("msgrcv","CONNECT");

    if (reply_from_server.client_id == -1){
        printf("Nie ma klienta o ID: %d!\n",other_id);
        return;
    }
    else if (reply_from_server.client_id == my_id){
        printf("Klient o ID: %d to Ty!\n",other_id);
        return;
    }
    else if (other_id != reply_from_server.client_id){
        printf("Klient %d aktualnie czatuje z klientem %d!\n",other_id, reply_from_server.client_id);
        return;
    }
    

    int other_queue = reply_from_server.queue_id;
    chat_mode(other_id, other_queue);
}

void initialize(){
    srand(time(NULL));
    char* home_path = getenv("HOME");
    char project_id = (char) rand() % 254 + 2;          //Wlasciwie mogloby zostac intem, ale zeby bylo zgodnie z tym co bylo 
    key_for_my_queue = ftok(home_path, project_id);     //podane w materialach to konwertuje na char
    if (key_for_my_queue == -1)
        err_exit_ftok();

    my_queue_id = msgget(key_for_my_queue, IPC_CREAT|0666);
    if (my_queue_id == -1)
        err_exit_msgget();

    if (atexit(stop) != 0){         // Obsluga wyjsca
        fprintf(stderr, "Nie udalo sie ustawic poprawnej funkcji zamykajcej program klienta!\n");
        exit(5);
    }
    
    key_t server_queue_key = ftok(home_path, SERVER_PROJECT_ID);
    if (server_queue_key == -1)
        err_exit_ftok();

    server_queue_id = msgget(server_queue_key, 0);      //0 czyli bez flag
    if (server_queue_id == -1)
        err_exit_msgget();

    my_msg msg;
    msg.msg_type = INIT;
    msg.queue_id = my_queue_id;
    if (msgsnd(server_queue_id, &msg, MSG_SIZE, 0) == -1) 
        err_exit_msg("msgsnd","INIT");

    my_msg reply_from_server;
    if (msgrcv(my_queue_id, &reply_from_server, MSG_SIZE, INIT, 0) == -1)
        err_exit_msg("msgsnd","INIT");
        
    my_id = reply_from_server.client_id;        // W client_id serwer odeslal info o przydzielonym ID
    if (my_id == -1){
        fprintf(stderr,"Serwer jest pelny! Nie mozna sie polaczyc\n");
        printf("Koniec dzialania\n");
        exit(42);
    }
    printf("Poprawnie podlaczono do serwera: %d\n",server_queue_id);
    printf("ID klienta: %d\n",my_id);
    printf("Numer kolejki: %d\n",my_queue_id);
}
///////////////////

// Inne
void chat_mode(int other_id, int other_queue){
    printf("CZAT ROZPOCZETY Z KLIENTEM %d\n", other_id);
    printf("Mozesz sie rozlaczyc wpisujac DISCONNECT\n");
    char* msg_buf = NULL;
    size_t size = 0;
    ssize_t read_bytes;
    while(1){
        my_msg other_info;

        if (msgrcv(my_queue_id, &other_info, MSG_SIZE, STOP, IPC_NOWAIT) >= 0){
            printf("Serwer zakonczyl swoje dzialanie - STOP\n");
            if (msg_buf != NULL)    free(msg_buf);
            exit(0);        // Wywoluje stop()
        }

        while (msgrcv(my_queue_id, &other_info, MSG_SIZE, CONNECT, IPC_NOWAIT) >=0){
            printf("Klient %d: %s\n", other_info.client_id, other_info.text);
        }

        if (msgrcv(my_queue_id, &other_info, MSG_SIZE, DISCONNECT, IPC_NOWAIT) >= 0){
            printf("Klient %d sie rozlaczyl\n", other_id);
            printf("--- CZAT ZAKONCZONY ---\n");
            break;      // Chyba najbezpieczniejsze wyjscie z programu
        }
        
        printf("> ");
        read_bytes = getline(&msg_buf, &size, stdin);
        my_msg msg;

        if (read_bytes > MAX_MSG_LENGTH)  
            printf("--- ZBYT DLUGA WIADOMOSC - PRZYCIETO ---\n");
        msg_buf[read_bytes-1] = '\0';
        if (strcmp(msg_buf,"DISCONNECT") == 0){
            printf("Rozlaczam sie z klientem %d\n",other_id);
            while (msgrcv(my_queue_id, &other_info, MSG_SIZE, CONNECT, IPC_NOWAIT) >=0) // Wyprozniamy kolejke z wiadomosci ktore zostaly
                printf("Klient %d: %s\n", other_info.client_id, other_info.text);
            
            disconnect();
            printf("--- CZAT ZAKONCZONY ---\n");
            break;
        }
        else if (strcmp(msg_buf,"") != 0){      // Nie chcemy wysylac pustej wiadomosci
            strcpy(msg.text, msg_buf);
            msg.client_id = my_id;
            msg.queue_id = my_queue_id;
            msg.msg_type = CONNECT;
            if (msgsnd(other_queue, &msg, MSG_SIZE, 0) == -1)
                err_exit_msg("msgsnd","CHAT");
        }
    }
    if (msg_buf != NULL)
        free(msg_buf);
}

void handle_SIGINT(int sig){
    exit(0);
}

void check_messages(){
    my_msg msg;
    if  (msgrcv(my_queue_id, &msg, MSG_SIZE, 0, IPC_NOWAIT) == -1)
        return;     // To w przypadku gdy nie ma zadnej wiadomosci od serwera

    // Tutaj dojedziemy tylko, jesli mamy wiadomosc od serwera
    if (msg.msg_type == CONNECT){
        int other_id = msg.client_id;
        int other_queue = msg.queue_id;        // Zakladamy, ze w queue_id jest idetyfikator kolejki drugiego klienta
        printf("Klient %d chce sie z Toba polaczyc!\n", other_id);
        chat_mode(other_id, other_queue);
    }
    else{       // Drugi komunikat jaki serwer moze tutaj wyslac to koniec dzialania 
        printf("Serwer %d zakonczyl swoje dzialanie, nastepuje rozlaczanie...\n", server_queue_id);
        exit(0);    // Wywoluje stop
    }
}

int main(){
    signal(SIGINT, handle_SIGINT);
    initialize();

    printf("Dostepne komendy: STOP CONNECT LIST\n");
    printf("Czekam na komende\n");
    
    char command_buf[MAX_MSG_LENGTH];
    while(1){
        printf("> ");
        check_messages();
        // Jakies wciagnie 
        scanf("%s", command_buf);

        if ( strcmp(command_buf, "STOP") == 0 ){
            exit(0);
        }
        else if ( strcmp(command_buf, "CONNECT") == 0 ){
            char other_client[MAX_MSG_LENGTH];
            scanf("%s", other_client);
            if ( !isANumber(other_client) || atoi(other_client) <= 0)
                printf("ID klienta do czatowania musi byc liczba dodatania!\n");
            int other_id = atoi(other_client);
            connect(other_id);

        }
        else if ( strcmp(command_buf, "LIST") == 0){
            list();
        }
        else{
            printf("Nieznana komenda %s!\n",command_buf);
            printf("Dostepne komendy: STOP CONNECT LIST\n");
        }
    }

    return 0;
}