#include "utils.h"
#include "sendData.h"

int SOCK = -1;   // socket
int epoll_fd = -1;
int send_disc_msg = 1;
GameStatus client_board;
Player players[2];

char* myName(){
    return players[0].player_name;
}

char myMarker(){
    return players[0].marker;
}

char* opponentName(){
    return players[1].player_name;
}

char opponentMarker(){
    return players[1].marker;
}

int unix_connection(char* server_socket_path, char* player_name){
    struct sockaddr_un unix_addr, bind_addr;       
    unix_addr.sun_family = bind_addr.sun_family = AF_UNIX;
    if (strlen(server_socket_path) > UNIX_PATH_MAX)
        errExit("Sciezka do gniazda servera jest zbyt dluga!", "w unix_connection()", 3);

    sprintf(bind_addr.sun_path,"/tmp/%ld%s", time(NULL), player_name);
    strcpy(unix_addr.sun_path, server_socket_path);
    int sock = socket(AF_UNIX, COMMUNICATION_TYPE, 0);

    if (sock == -1)
        errExit("Wystapil problem z tworzenia gniazda dla klienta!", "w unix_connection()", 4);
    if (bind(sock, (struct sockaddr*) &bind_addr, sizeof(bind_addr)) == -1)
        errExit("Wystapil problem z bindowaniem gniazda!","w unix_connection()", 5);
    if (connect(sock, (struct sockaddr*) &unix_addr, sizeof(unix_addr)) < 0)
        errExit("Wystapil problem z polaczeniem z serwerem!", "w unix_connection()", 5);
    return sock;
}

int ipv4_connection(char* server_addres, int port){
    struct sockaddr_in ipv4_addr;
    ipv4_addr.sin_family = AF_INET;
    ipv4_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_addres, &ipv4_addr.sin_addr) < 0)
        errExit("Podany adres jest niepoprawny!","w ipv4_connection()", 1);

    int sock = socket(AF_INET, COMMUNICATION_TYPE, 0);

    if (sock == -1)
        errExit("Wystapil problem z tworzenia gniazda dla klienta!", "w ipv4_connection()", 4);
    if (connect(sock, (struct sockaddr*) &ipv4_addr, sizeof(ipv4_addr)) < 0)
        errExit("Wystapil problem z polaczeniem z serwerem!", "w ipv4_connection()", 5);
    return sock;
}

void drawBoard(int incorrect_input){
    system("clear");            // DROGA NA SKROTY

    // POTEZNY PRINTF WYPISUJACY TABELE
    printf("\n\n");
    for(int i=0; i < 2; i++){
        printf("%c|%c|%c\n", client_board.game_board[3*i], client_board.game_board[3*i+1], client_board.game_board[3*i+2]);
        printf("-----\n");
    }
    printf("%c|%c|%c\n", client_board.game_board[6], client_board.game_board[7], client_board.game_board[8]);
    printf("\n\n");
    printf("Twoj znak: %c\n", myMarker());
        printf("Grasz przeciwko: %s\n\n", opponentName());

    if (client_board.current_player == myMarker()){
        if (incorrect_input == 1)
            printf("Podano niepoprawne pole!\n");
        else if (incorrect_input == 2)
            printf("Podane pole jest juz zajete!\n");
        printf("Twoj ruch (1-9): ");
    }
    else{
        printf("Teraz ruch %s.\n",opponentName());
        if (incorrect_input == 3)
            printf("Teraz ruch przeciwnika!\n");
    }
}

void myMove(){
    int my_move = -1;
    if (scanf("%d",&my_move) != 1){         
        char buf = 'a';

        while((buf = getchar()) != EOF && buf != EOF && buf != '\n');       // Pomijamy caly input
        int incorrect_input = 1;
        drawBoard(incorrect_input);
    }
    else{
        if (my_move >= 1 && my_move <= 9){
            my_move--;
            if (client_board.current_player != myMarker()){
                int not_my_turn = 3;
                drawBoard(not_my_turn);
            }
            else{
                if (client_board.game_board[my_move] != myMarker() && client_board.game_board[my_move] != opponentMarker()){
                    client_board.game_board[my_move] = myMarker();
                    drawBoard(0);
                    Msg msg;
                    msg.evnt = evnt_move;
                    msg.game_status = client_board;
                    write(SOCK, &msg, sizeof(msg));
                }
                else{
                    int already_used_cell = 2;
                    drawBoard(already_used_cell);
                }
            }
        }
        else{
            int incorrect_input = 1;
            drawBoard(incorrect_input);
        }
    }
}

void handleEvent(struct epoll_event event){
    if (event.data.fd == STDIN_FILENO){
        myMove();
    }
    else{
        Msg received_msg;
        read(SOCK, &received_msg, sizeof(received_msg));        
        switch (received_msg.evnt){
            case evnt_waiting:
                printf("Oczekiwanie na drugiego gracza...\n");
                break;

            case evnt_full_server:
                printf("Serwer jest pelny!\n");
                send_disc_msg = 0;
                exit(0);

            case evnt_name_taken:
                printf("Nazwa uzytkownika jest juz zajeta!\n");
                exit(0);

            case evnt_game_over:
                if ( received_msg.winner == '=')
                    printf("\nREMIS :O\n");
                else
                    printf("\n%s\n", (received_msg.winner) == myMarker() ? "WYGRALES ^^" : "Przegrales ;___;");
                exit(0);   
            
            case evnt_ping:
                write(SOCK, &received_msg, sizeof(received_msg));       // Najprosciej jest odeslac ta sama wiadomosc
                break;

            case evnt_update:
                client_board = received_msg.game_status;
                printf("-> %s|%c\n", received_msg.game_status.game_board, received_msg.game_status.current_player);
                drawBoard(0);
                break;

            case evnt_start_game:
                memcpy(&players[1], &received_msg.player_info, sizeof(players[1]));
                players[0].marker = (opponentMarker() == 'O') ? 'X' : 'O';
                drawBoard(0);
                break;

            case evnt_opponent_disconnect:
                printf("\nPrzeciwnik sie rozlaczyl.\n");
                exit(0);

            case evnt_disconnect:
                printf("\nSerwer zostal zamkniety - nastapilo rozlaczenie.\n");
                exit(0);

            default:
                break;
        }
        if (event.events & EPOLLHUP){     // Nagle zerwanie polaczenia
            printf("\nPolaczenie z serwerem zostalo zerwane!\n");
            exit(0);
        }
    }
}

void quitNotifier(){
    if (send_disc_msg){
        Msg msg;
        msg.evnt = evnt_disconnect;
        write(SOCK, &msg, sizeof(msg));
    }
    if (epoll_fd != -1)
        close(epoll_fd);
    if (SOCK != -1)
        close(SOCK);
}

void handleSIGINT(int signo){
    exit(0);
}
///////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv){
    char needed_args[] = "Potrzebne argumenty: nazwa_gracza local|inet sciezka_gniazda/[adres_serwera  numer_portu] (zalezy od drugiego argumentu)";

    if (argc != 4 && argc != 5)
        errExit("Niepoprawna liczba argumentow!", needed_args, 2);

    strcpy(players[0].player_name, argv[1]);       // Nazwa gracza
    char* connection_type = argv[2];
    if ( strcmp(connection_type, "local") == 0){
        char* server_socket_path = argv[3];
        SOCK = unix_connection(server_socket_path, myName());
    }
    else if( strcmp(connection_type, "inet") == 0 ){
        char* server_addres = argv[3];
        int port = (isANumber(argv[4])) ? atoi(argv[4]) : -1;
        if (port == -1)
            errExit("Port musi byc liczba!", "W main()", 2);
        SOCK = ipv4_connection(server_addres, port);
    }
    
    if (atexit(quitNotifier) != 0)
        errExit("Nie udalo sie zarejestrowac poprawnej funkcji do wywolania na wyjsciu!", "W main()", 2);
    if (signal(SIGINT, handleSIGINT) == SIG_ERR)
        errExit("Nie udalo sie zarejestrowac poprawnej funkcji do obslugi sygnalu SIGINT!","W main()", 2);
    

    epoll_fd = epoll_create1(0);         // Bez flag
    if (epoll_fd == -1)
        errExit("Nie udalo sie uzyskac deskryptora dla mechanizmu epoll!","W main()", 2);

    struct epoll_event epoll_stdin_event;
    epoll_stdin_event.events = EPOLLIN|EPOLLPRI;
    epoll_stdin_event.data.fd = STDIN_FILENO;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &epoll_stdin_event) == -1)
        errExit("Nie udalo sie zarejestrowac standardowego wejscia w mechanizmie EPOLL!","W main()", 12);

    struct epoll_event epoll_sock_event;
    epoll_sock_event.events = EPOLLIN|EPOLLPRI|EPOLLHUP;
    epoll_sock_event.data.fd = SOCK;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, SOCK, &epoll_sock_event) == -1)
        errExit("Nie udalo sie zarejestrowac gniazda w mechanizmie EPOLL!","W main()", 12);
    
    Msg msg;
    msg.evnt = evnt_init;
    strcpy(msg.my_name, myName());
    send(SOCK, &msg, sizeof(msg), 0);

    struct epoll_event pending_events[MAX_PENDING_EVENTS];

    while(1){
        int event_count = epoll_wait(epoll_fd, pending_events, MAX_PENDING_EVENTS, -1); 
        for(int i=0; i < event_count; i++){
            struct epoll_event event = pending_events[i];
            handleEvent(event);
        }
    }

    return 0;
}