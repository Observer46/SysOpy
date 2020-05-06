#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>
#include <mqueue.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>


#include "posix_chat_settings.h"

int my_id;
mqd_t my_queue_id;
mqd_t server_queue_id;
char my_queue_name[QUEUE_NAME_LENGTH];


void chat_mode();       // Zadeklarowane dla funkcji connect()

int isANumber(char* number){
    for(int i=0; i < strlen(number); i++)
        if ( !isdigit(number[i]) )
            return 0;
    return 1;
}

void generate_queue_name(){
    char queue_name[QUEUE_NAME_LENGTH];
    queue_name[0] = '/';
    for(int i=1; i < QUEUE_NAME_LENGTH; i++)
        queue_name[i] = (char) (rand() % 25) + 65;        // Dbamy o wzgledna czytelnosc nazwy
    strcpy(my_queue_name, queue_name);
    //my_queue_name[strlen(my_queue_name) - 1]; //= '\0';
}

// Bledy
void err_exit_msg(char* function_name, char* msg_type){
    fprintf(stderr,"Wystapil problem z obsluga komunikatu %s w funkcji %s!\n", msg_type, function_name);
    exit(1);
}

void err_exit_del_queue(){
    fprintf(stderr,"Wystapil problem z usuwaniem kolejki %s u klienta %d\n", my_queue_name, my_id);
    exit(2);
}

void err_exit_close_queue(mqd_t queue_id){
    fprintf(stderr,"Wystapil problem z zamknieciem kolejki %d u klienta %d\n", queue_id, my_id);
    exit(3);
}

void err_exit_mq_open(const char* queue_name){
    fprintf(stderr,"Nie udalo sie uzyskac kolejki %s przy pomocy funkcji mq_open()!\n", queue_name);
    exit(4);
}
///////////////////////////////////////

// Obsluga komunikatow
void stop(){
    char msg[MAX_MSG_LENGTH];
    //msg[0] = (char) my_id;  //Info o 

    if (mq_send(server_queue_id, msg, MAX_MSG_LENGTH, STOP) == -1)      // Najpierw wysylamy informacje do serwera
        printf("Nie udao sie wyslac wiadomosci konczacej do serwera!\n");                            

    if (mq_close(server_queue_id) == -1)
        err_exit_close_queue(server_queue_id);

    if (mq_close(my_queue_id) == -1)
        err_exit_close_queue(my_queue_id);

    if (mq_unlink(my_queue_name) == -1)
        err_exit_del_queue();

    printf("\n**Koniec dzialania klienta**\n");
}

void disconnect(mqd_t other_queue){
    char msg[MAX_MSG_LENGTH];
    if (mq_send(server_queue_id, msg, MAX_MSG_LENGTH, DISCONNECT) == -1)
        err_exit_msg("mq_send()","DISCONNECT");
    if (mq_close(other_queue) == -1)
        err_exit_close_queue(other_queue);
}

void list(){
    char msg[MAX_MSG_LENGTH];
    if (mq_send(server_queue_id, msg, MAX_MSG_LENGTH, LIST) == -1)
        err_exit_msg("mq_send()","LIST");
    
    char reply_from_server[MAX_MSG_LENGTH];
    unsigned prio;
    if (mq_receive(my_queue_id, reply_from_server, MAX_MSG_LENGTH, &prio) == -1)
        err_exit_msg("mq_receive()","LIST");
    if (prio != LIST)
        err_exit_msg("mq_receive()","LIST - zly komunikat");
    printf("\tLista dostepnych klientow:\n%s",reply_from_server);
}

void connect(int other_id){
    char msg[MAX_MSG_LENGTH-1];
    strcpy(msg,"");
    msg[0] = (char) other_id;   // tak przesylamy info o tym z kim chcemy sie polaczyc
    strcat(msg, my_queue_name);
    if (mq_send(server_queue_id, msg, MAX_MSG_LENGTH, CONNECT) == -1)
        err_exit_msg("mq_send()","CONNECT");
    
    char reply_from_server[MAX_MSG_LENGTH];
    unsigned prio;
    if (mq_receive(my_queue_id, reply_from_server, MAX_MSG_LENGTH, &prio) == -1)
        err_exit_msg("mq_receive()","CONNECT");
    if (prio != CONNECT)
        err_exit_msg("mq_receive()","CONNECT - zly komunikat");

    int client_id = (int) reply_from_server[0];
    if (client_id == -1){
        printf("Nie ma klienta o ID: %d!\n",other_id);
        return;
    }
    else if (client_id == my_id){
        printf("Klient o ID: %d to Ty!\n",other_id);
        return;
    }
    else if (other_id != client_id){
        printf("Klient %d aktualnie czatuje z klientem %d!\n",other_id, client_id);
        return;
    }
    
    char other_queue_name[MAX_MSG_LENGTH];
    strcpy(other_queue_name, reply_from_server + 1); // Zaczynamy kopiowanie od drugiego znaku, bo tam zaczyna sie nazwa
    mqd_t other_queue = mq_open(other_queue_name, O_RDWR,0666, NULL);
    if (other_queue == -1)
        err_exit_mq_open(other_queue_name);

    chat_mode(other_id, other_queue);
}

void initialize(){
    srand(time(NULL));
    //struct mq_attr attributes;
    // attributes.mq_maxmsg = 16;
    // attributes.mq_msgsize = MAX_MSG_LENGTH;
    generate_queue_name();
    my_queue_id = mq_open(my_queue_name, O_RDONLY|O_CREAT,0666, NULL);//attributes);
    if (my_queue_id == -1){
        // if (errno == EACCES)
        //     printf("EACCES\n");
        // if(errno == EEXIST)
        //     printf("EEXIST\n");
        // if(errno == EINVAL)
        //     printf("EINVAL\n");
        // if (errno == ENAMETOOLONG)
        //     printf("ENAMETOOLONG\n");
        // if(errno == ENOSPC)
        //     printf("ENOSPC\n");
        // if (errno == ENOMEM)
        //     printf("ENOMEM\n");
        // if (errno == ENOENT)
        //     printf ("ENOENT\n");
        // printf("%d\n",errno);   
        err_exit_mq_open(my_queue_name);
    }
    printf("Nazwa mojej kolejki: %s\n",my_queue_name);

    if (atexit(stop) != 0){         // Obsluga wyjsca
        fprintf(stderr, "Nie udalo sie ustawic poprawnej funkcji zamykajcej program klienta!\n");
        exit(5);
    }
    my_queue_id = mq_open(my_queue_name, O_RDONLY|O_CREAT,0666, NULL);
    if (my_queue_id == -1){  
        err_exit_mq_open(my_queue_name);
    }
    server_queue_id = mq_open(SERVER_NAME, O_WRONLY, 0666, NULL);     
    if (server_queue_id == -1)
        err_exit_mq_open(SERVER_NAME);
    
    
    char msg[MAX_MSG_LENGTH];
    strcpy(msg, my_queue_name);
    if (mq_send(server_queue_id, msg, MAX_MSG_LENGTH, INIT) == -1){
        
        err_exit_msg("mq_send()","INIT");
    }

    char reply_from_server[MAX_MSG_LENGTH];
    unsigned prio;
    if (mq_receive(my_queue_id, reply_from_server, MAX_MSG_LENGTH, &prio) == -1)
        err_exit_msg("mq_receive()","INIT");
    if (prio != INIT)
        err_exit_msg("mq_receive()","INIT - zly komunikat");
        
    my_id = (int) reply_from_server[0];        // W pierszym bajcie serwer odeslal info o przydzielonym ID
    if (my_id == 0){      // 255 oznacza, ze nie ma miejsca
        fprintf(stderr,"Serwer jest pelny! Nie mozna sie polaczyc\n");
        printf("Koniec dzialania\n");
        exit(42);
    }
    printf("Poprawnie podlaczono do serwera: %s\n",SERVER_NAME);
    printf("ID klienta: %d\n",my_id);
    printf("Nazwa kolejki: %s\n",my_queue_name);
}
///////////////////

// Inne
void chat_mode(int other_id, mqd_t other_queue){
    printf("CZAT ROZPOCZETY Z KLIENTEM %d\n", other_id);
    printf("Mozesz sie rozlaczyc wpisujac DISCONNECT\n");
    char* msg_buf = NULL;
    size_t size = 0;
    ssize_t read_bytes;
    while(1){
        char other_info[MAX_MSG_LENGTH];
        unsigned prio;
        struct timespec timeout;
        timeout.tv_nsec = 100;
        while (mq_timedreceive(my_queue_id, other_info, MAX_MSG_LENGTH, &prio, &timeout) >= 0){   // Aby nie czakac caly czas na wiadomosc
            if(prio == STOP){
                printf("Serwer zakonczyl swoje dzialanie - STOP\n");
                if (msg_buf != NULL)    free(msg_buf);
                exit(0);        // Wywoluje stop()
            }
            else if (prio == DISCONNECT){
                printf("Klient %d sie rozlaczyl\n", other_id);
                printf("--- CZAT ZAKONCZONY ---\n");
                if (mq_close(other_queue) == -1)
                    err_exit_close_queue(other_queue);
                break;      // Chyba najbezpieczniejsze wyjscie z czatu
            }
            else{
                printf("Klient %d: %s\n", (int) other_info[0], other_info + 1);   // +1 oznacza, ze bez pierwszego znaku
            }
        }

        // while (mq_timedreceive(my_queue_id, other_info, MAX_MSG_LENGTH, &prio, &timeout) >=0){
            
        // }

        // if (msgrcv(my_queue_id, &other_info, MAX_MSG_LENGTH, DISCONNECT, IPC_NOWAIT) >= 0){
            
        // }
        
        printf("> ");
        read_bytes = getline(&msg_buf, &size, stdin);
        char msg[MAX_MSG_LENGTH];

        if (read_bytes > MAX_MSG_LENGTH)  
            printf("--- ZBYT DLUGA WIADOMOSC - PRZYCIETO ---\n");
        msg_buf[read_bytes-1] = '\0';
        if (strcmp(msg_buf,"DISCONNECT") == 0){
            printf("Rozlaczam sie z klientem %d\n",other_id);
            while (mq_timedreceive(my_queue_id, other_info, MAX_MSG_LENGTH, &prio, &timeout) >=0) // Wyprozniamy kolejke z wiadomosci ktore zostaly
                printf("Klient %d: %s\n", (int) other_info[0], other_info + 1);
            
            disconnect(other_queue);
            printf("--- CZAT ZAKONCZONY ---\n");
            break;
        }
        else if (strcmp(msg_buf,"") != 0){      // Nie chcemy wysylac pustej wiadomosci
            strcpy(msg, "");
            msg[0] = (char) my_id;
            strcat(msg, msg_buf);                //CAREFUL
            if (mq_send(other_queue, msg, MAX_MSG_LENGTH, CONNECT) == -1)
                err_exit_msg("mq_send()","CHAT");
        }
    }
    if (msg_buf != NULL)
        free(msg_buf);
}

void handle_SIGINT(int sig){
    exit(0);
}

void check_messages(){
    char msg[MAX_MSG_LENGTH];
    unsigned prio;
    struct timespec timeout;
    timeout.tv_nsec = 100;
    if  (mq_timedreceive(my_queue_id, msg, MAX_MSG_LENGTH, &prio, &timeout) == -1)
        return;     // To w przypadku gdy nie ma zadnej wiadomosci od serwera

    // Tutaj dojedziemy tylko, jesli mamy wiadomosc od serwera
    if (prio == CONNECT){
        int other_id = (int) msg[0];
        char other_queue_name[MAX_MSG_LENGTH];      
        strcpy(other_queue_name, msg + 1);      // Bez pierwszego znaku
        printf("Klient %d chce sie z Toba polaczyc!\n", other_id);
        mqd_t other_queue = mq_open(other_queue_name, O_RDWR,0666, NULL);
        if (other_queue == -1)
            err_exit_mq_open(other_queue_name);
        chat_mode(other_id, other_queue);
    }
    else{       // Drugi komunikat jaki serwer moze tutaj wyslac to koniec dzialania 
        printf("Serwer %s zakonczyl swoje dzialanie, nastepuje rozlaczanie...\n", SERVER_NAME);
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