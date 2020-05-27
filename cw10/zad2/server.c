#include "utils.h"
#include "sendData.h"
#include <pthread.h>

#define WIN_COND_COUNT 8

pthread_mutex_t server_mutex = PTHREAD_MUTEX_INITIALIZER;    // Inicjalizacja przy pomocy makr
int epoll_fd = -1;
int ipv4_socket = -1;
int local_socket = -1;
char* socket_path = NULL;

int player_count = 0;
Address test[MAX_PLAYERS_ON_SERVER];

Client* clients_on_server[MAX_PLAYERS_ON_SERVER];
Client* client_waiting_for_opponent = NULL;


// Do debugowania 
/////////////////////////
void print_addr(Address client_addr){
    struct sockaddr_in* in = (struct sockaddr_in*) &client_addr;
    struct sockaddr_un* un = (struct sockaddr_un*) &client_addr;
    printf("Family: %s\n", (in->sin_family == AF_INET) ? "AF_INET" : "AF_UNIX");
    printf("AF_UNIX: %s\n", un->sun_path);
    printf("AF_INET: %s:::%d\n", inet_ntoa(in->sin_addr), in->sin_port);
}
/////////////////////

int win_condition[WIN_COND_COUNT][3] = {   {0, 1, 2}, {3, 4, 5}, {6, 7, 8},
                            {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
                            {0, 4, 8}, {2, 4, 6} };     // Sekewnecje konczace gre

int matchDrawCheck(GameStatus* match){
    for(int i=0; i < 9; i++)
        if (match->game_board[i] != 'X' && match->game_board[i] != 'O')
            return 0;
    return 1;
}

int isGameOver(Client* client){
    if (matchDrawCheck(client->match))
        return 2;
    char marker_to_check = client->player_info.marker;
    char* match = client->match->game_board;
    for(int i=0; i < WIN_COND_COUNT; i++){
        int win = 1;
        for(int j=0; j < 3; j++)        // 3 pozycje na ktorych potencjalnie mozna wygrac
            if (match[win_condition[i][j]] != marker_to_check)
                win = 0;
        if (win)
            return 1;
    }
    return 0;
}

int nameTaken(char* name){
    for(int i=0; i < MAX_PLAYERS_ON_SERVER; i++){
        Client* client = clients_on_server[i];
        if (client != NULL  && (strcmp(client->player_info.player_name, name) == 0) )
            return 1;
    }
    return 0;
}

int findEmptySlot(){
    int idx;
    for(idx=0; idx < MAX_PLAYERS_ON_SERVER && clients_on_server[idx] != NULL; idx++);
    return (idx < MAX_PLAYERS_ON_SERVER) ? idx : -1;        // -1 kiedy nie ma wolnego miejsca
}

int findClientSlot(Address address, socklen_t adr_size){
    int idx;
    for(idx=0; idx < MAX_PLAYERS_ON_SERVER; idx++)
        if(clients_on_server[idx] != NULL)
            if (memcmp(&clients_on_server[idx] -> address, &address, adr_size) == 0)
                break;
    return (idx < MAX_PLAYERS_ON_SERVER) ? idx : -1;        // -1 kiedy nie ma takiego gracza
}

void removeClientConnection(Client* client, int notify, int quit_cleanup){
    printf("Serwer: Usuwam gracza %s\n",client->player_info.player_name);
    Client* opponent = NULL;

    int my_idx = findClientSlot(client -> address, client->addr_len);
    clients_on_server[my_idx] = NULL;

    if (client == client_waiting_for_opponent)  
        client_waiting_for_opponent = NULL;
    
    if (client->opponent != NULL){
        opponent = client->opponent;
        client->opponent->opponent = NULL;      // Nie chcemy, aby tamten probowal nas usunac jak juz jestesmy usunieci
        if(notify){
            Msg msg;
            msg.evnt = evnt_opponent_disconnect;
            sendto(opponent->socket_fd, &msg, sizeof(msg), 0, (struct sockaddr*) &opponent->address, opponent->addr_len);
        }
        if (client->match != NULL)
            free(client->match);
    }

    if (opponent != NULL && !quit_cleanup)
        removeClientConnection(opponent, 0, quit_cleanup);
    free(client);
}

void* pingRoutine(void* arg){
    Msg ping_msg;
    ping_msg.evnt = evnt_ping;
    while(1){
        sleep(PING_TIMER);
        printf("Serwer: Ping\n");
        pthread_mutex_lock(&server_mutex);
        for(int i=0; i < MAX_PLAYERS_ON_SERVER; i++){
            Client* player = clients_on_server[i];
            if (player != NULL){
                if (player->is_responding){
                    player->is_responding = 0;       // Musi odpowiedziec by uznac go za odpowiadajacy
                    printf("Wysylam do: %s\n", player->player_info.player_name);
                    if (sendto(player->socket_fd, &ping_msg, sizeof(ping_msg), 0, (struct sockaddr*) &player->address, player->addr_len) == -1)
                        errExit("Serwer: Wysylanie zawalilo!","W pingRoutine()", 13);
                }
                else{
                    printf("Serwer: Pingowy timeout dla %s\n", player->player_info.player_name);
                    removeClientConnection(player, 1, 0);
                }
            }
        }
        pthread_mutex_unlock(&server_mutex);
    }
}

void sendMatchStatusToClient(Client* client){
    Msg msg;
    msg.evnt = evnt_update;
    strcpy(msg.game_status.game_board, client->match->game_board);
    char cur_player = client->match->current_player;
    msg.game_status.current_player = cur_player;
    sendto(client->socket_fd, &msg, sizeof(msg), 0, (struct sockaddr*) &client->address, client->addr_len);
}

void initBoard(GameStatus* match){
    match->game_board[0] = '1';
    match->game_board[1] = '2';
    match->game_board[2] = '3';
    match->game_board[3] = '4';
    match->game_board[4] = '5';
    match->game_board[5] = '6';
    match->game_board[6] = '7';
    match->game_board[7] = '8';
    match->game_board[8] = '9';
}

void pairWithWaitingClient(Client* pair_for_waiting){
    GameStatus* new_match = (GameStatus*)calloc(1,sizeof(GameStatus));
    pair_for_waiting -> match = new_match;
    client_waiting_for_opponent -> match = new_match;
    client_waiting_for_opponent -> opponent = pair_for_waiting;
    pair_for_waiting -> opponent = client_waiting_for_opponent;

    int waiting_is_X = rand() % 2;
    if (waiting_is_X){
        client_waiting_for_opponent -> player_info.marker = 'X';
        pair_for_waiting -> player_info.marker = 'O';
    }
    else{
        client_waiting_for_opponent -> player_info.marker = 'O';
        pair_for_waiting -> player_info.marker = 'X';
    }

    Msg msg;
    msg.evnt = evnt_start_game;
    memcpy(&msg.player_info, &pair_for_waiting -> player_info, sizeof(msg.player_info));
    sendto(client_waiting_for_opponent -> socket_fd, &msg, sizeof(msg), 0 ,(struct sockaddr*) &client_waiting_for_opponent->address, client_waiting_for_opponent->addr_len);

    memcpy(&msg.player_info, &client_waiting_for_opponent -> player_info, sizeof(msg.player_info));
    sendto(pair_for_waiting -> socket_fd, &msg, sizeof(msg), 0, (struct sockaddr*) &pair_for_waiting -> address, pair_for_waiting -> addr_len);

    initBoard(pair_for_waiting->match);
    int waiting_goes_first = rand() % 2;
    if (waiting_goes_first)
        pair_for_waiting->match->current_player = client_waiting_for_opponent -> player_info.marker;
    else
        pair_for_waiting->match->current_player = pair_for_waiting -> player_info.marker;

    sendMatchStatusToClient(client_waiting_for_opponent);
    sendMatchStatusToClient(pair_for_waiting);
    client_waiting_for_opponent = NULL; 
}

Client* newClientConnection(int client_socket_fd, Address address, socklen_t addr_len){
    int idx = findEmptySlot();
    if (idx == -1)
        return NULL;

    Client* new_client = (Client*) calloc(1,sizeof(Client));
    clients_on_server[idx] = new_client;
    new_client -> socket_fd = client_socket_fd;
    new_client -> is_responding = 1;
    new_client -> address = address;
    new_client -> addr_len = addr_len;
    test[player_count] = address;
    player_count++;
    return clients_on_server[idx];    
}

void socketSetup(int sock, void* address, int address_size){        // Przez void* nie ma informacji o rozmiarze 'address'
    if (bind(sock, (struct sockaddr*) address, address_size) == -1)
        errExit("Serwer: Nie udalo sie poprawnie zainicjalizowac socketu!","w socketSetup()", 1);
    
    struct epoll_event connection_event;
    connection_event.events = EPOLLIN|EPOLLPRI;
    connection_event.data.fd = sock;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &connection_event) == -1)
        errExit("Serwer: Nie udalo sie zarejestrowac gniazda w mechanizmie EPOLL!","W socketSetup()", 1);
    
}

void handleClientMsg(Client* client, Msg msg){
    switch(msg.evnt){
        case evnt_ping:
            printf("Serwer: Gracz %s odpowiedzial na ping.\n", client->player_info.player_name);
            client->is_responding = 1;
            break;

        case evnt_move:
        {
            GameStatus current_status = msg.game_status;
            current_status.current_player = client -> opponent -> player_info.marker;
            *client -> match = current_status;

            sendMatchStatusToClient(client -> opponent);
            sendMatchStatusToClient(client);

            int match_result = isGameOver(client);
            if (match_result > 0){
                printf("Serwer: Gracze %s i %s zakonczyli rozgrywke!\n", client->player_info.player_name, client->opponent->player_info.player_name);
                int opponent_socket_fd = client->opponent->socket_fd;
                Address opponent_address = client->opponent->address;
                socklen_t opponent_addr_len = client->opponent->addr_len;
                client->opponent->opponent = NULL;      // Dzieki temu uzyskamy wiadomosc o koncu gry a nie o zerwaniu polaczenia
                client->opponent = NULL;                // Bez tego dojdzie do jednego usuwanie wiecej niz potrzeba
                Msg response;
                response.evnt = evnt_game_over;
                response.winner = (match_result == 1) ? client -> player_info.marker : '=';

                sendto(opponent_socket_fd, &response, sizeof(response), 0, (struct sockaddr*) &opponent_address, opponent_addr_len);
                sendto(client->socket_fd, &response, sizeof(response), 0, (struct sockaddr*) &client->address, client->addr_len);
            }
            break;
        }

        case evnt_disconnect:
            removeClientConnection(client, 1, 0);
            break;
        
        default:
            printf("Serwer: Otrzymano nieznany komunikat!\n");
            sendMatchStatusToClient(client);
    }
}

void handleServerEvent(struct epoll_event event){
    int client_socket_fd = event.data.fd;
    Msg received;
    Address client_addr;
    socklen_t addr_len = sizeof(client_addr);
    recvfrom(client_socket_fd, &received, sizeof(received), 0, (struct sockaddr*) &client_addr, &addr_len);
    if (received.evnt == evnt_init){       // Ktorys socket daje znac, ze dostal nowe polaczenie;
        printf("Serwer: Nowe polaczenie!\n");
        Client* client = newClientConnection(client_socket_fd, client_addr, addr_len);
        if (client == NULL){
            printf("Serwer: Proba podlaczenia do pelnego serwera.\n");
            Msg msg;
            msg.evnt = evnt_full_server;
            sendto(client_socket_fd, &msg, sizeof(msg), 0, (struct sockaddr*) &client_addr, addr_len);
        }
        else{
            char* client_name = received.my_name;
            if (nameTaken(client_name)){
                Msg response;
                response.evnt = evnt_name_taken;
                sendto(client->socket_fd, &response, sizeof(response), 0, (struct sockaddr*) &client->address, client->addr_len);
                printf("Serwer: Nazwa %s jest juz zajeta!\n", client_name);
            }
            else{
                strcpy(client -> player_info.player_name, client_name);
                printf("Serwer: Dolaczyl nowy klient - %s\n", client -> player_info.player_name);
                if (client_waiting_for_opponent == NULL){
                    printf("Serwer: Klient %s czeka na drugiego gracza.\n", client_name);
                    Msg response;
                    response.evnt = evnt_waiting;
                    sendto(client->socket_fd, &response, sizeof(response), 0, (struct sockaddr*) &client->address, client->addr_len);
                    client_waiting_for_opponent = client;
                }
                else{
                    printf("Serwer: Laczenie %s i %s do gry!\n", client_name, client_waiting_for_opponent -> player_info.player_name);
                    pairWithWaitingClient(client);
                }
            }
        }
    }
    else{
        // printf("Serwer: Nadchodzacy adres:\n");      // Duzo spamu generuje
        // print_addr(client_addr);
        int idx = findClientSlot(client_addr, addr_len);
        if (idx != -1){
            Client* client = clients_on_server[idx];
            handleClientMsg(client, received);
        }
        else if (received.evnt == evnt_ping){
            printf("Serwer: Nieznany nadawca!\n");
        }
    }
}

void quitCleanUp(){
    printf("\nZamykanie serwera...\n");
    for (int i=0; i < MAX_PLAYERS_ON_SERVER; i++){
        Client* client = clients_on_server[i];
        if(client != NULL){
            printf("Serwer: Wysylam zamkniecie do %s\n", client->player_info.player_name);
            Msg msg;
            msg.evnt = evnt_disconnect;
            sendto(client->socket_fd, &msg, sizeof(msg), 0, (struct sockaddr*) &client->address, client->addr_len);
            int quit_cleanup = 1;
            removeClientConnection(client, 0, quit_cleanup);
        }
    }

    if (epoll_fd != -1)
        close(epoll_fd);

    pthread_mutex_destroy(&server_mutex);

    if (local_socket != -1)
        close(local_socket);
    if (ipv4_socket != -1)
        close(ipv4_socket);
    if (socket_path != NULL)     
        unlink(socket_path);
}

void handleSIGINT(int signo){
    exit(0);
}

int main(int argc, char** argv){
    srand(time(NULL));
    for(int i=0; i < MAX_PLAYERS_ON_SERVER; i++)
        clients_on_server[i] = NULL;
        
    if (atexit(quitCleanUp) != 0){
        pthread_mutex_destroy(&server_mutex);
        errExit("Nie udalo sie zarejestrowac poprawnej funkcji do wywolania na wyjsci!", "W main()", 2);
    }
    if (signal(SIGINT, handleSIGINT) == SIG_ERR)
        errExit("Nie udalo sie zarejestrowac poprawnej obslugi sygnalu SIGINT!", "W main()", 2);

    char needed_args[] = "Potrzebne argumenty: port sciezka_dla_gniazda";

    if (argc != 3)
        errExit("Niepoprawna liczba argumentow!", needed_args, 2);

    if ( !isANumber(argv[1]) || atoi(argv[1]) <= 0)
        errExit("Port musi byc dodatnia liczba!", needed_args, 2);

    int port = atoi(argv[1]);
    socket_path = argv[2];

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
        errExit("Nie udalo sie uzyskac dekryptora mechanizmu EPOLL!", "W main()", 2);

    struct sockaddr_un local_address;
    local_address.sun_family = AF_UNIX;
    strcpy(local_address.sun_path, socket_path);
    local_socket = socket(AF_UNIX, COMMUNICATION_TYPE, 0);
    if (local_socket == -1)
        errExit("Nie udalo sie stworzyc gniazda lokalnego!", "W main()", 2);
    socketSetup(local_socket, &local_address, sizeof(local_address));      // O ile rozmiar jest poprawnie przekazywany

    struct sockaddr_in ipv4_address;
    ipv4_address.sin_family = AF_INET;
    ipv4_address.sin_port = htons(port);
    ipv4_address.sin_addr.s_addr = htonl(INADDR_ANY);
    ipv4_socket = socket(AF_INET, COMMUNICATION_TYPE, 0);
    if (ipv4_socket == -1)
        errExit("Nie udalo sie stworzyc gniazda internetowego!", "W main()", 2);
    socketSetup(ipv4_socket, &ipv4_address, sizeof(ipv4_address));

    pthread_t pinging_thread;
    pthread_create(&pinging_thread, NULL, pingRoutine, NULL);

    printf("Serwer rozpoczal dzialanie!\n");
    printf("Polaczenie internetowe: adres - %d, port - %d\n", ipv4_address.sin_addr.s_addr, port);
    printf("Polaczenie lokalne: sciezka - %s\n", socket_path);

    struct epoll_event pending_events[MAX_SERVER_PENDING_EVENTS];
    while(1){
        int event_count = epoll_wait(epoll_fd, pending_events, MAX_SERVER_PENDING_EVENTS, -1);
        for(int i=0; i < event_count; i++){
            struct epoll_event event = pending_events[i];
            pthread_mutex_lock(&server_mutex);
            handleServerEvent(event);
            pthread_mutex_unlock(&server_mutex);
        }
    }
    
    
    return 0;
}