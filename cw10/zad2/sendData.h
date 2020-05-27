#include "utils.h"

typedef struct GameStatus{
    char game_board[9];
    char current_player; 
} GameStatus;

typedef enum EventType{
    evnt_init,
    evnt_disconnect,
    evnt_game_over,
    evnt_start_game,
    evnt_move,
    evnt_full_server,
    evnt_ping,
    evnt_name_taken,
    evnt_update,
    evnt_waiting,
    evnt_opponent_disconnect
} Event;

typedef struct PlayerInfo{
    char player_name[NAME_LENGTH];
    char marker;
} Player;

typedef struct Message{
    Event evnt;
    GameStatus game_status;
    Player player_info;
    char winner;        // X, O oraz = (= to remis)
    char my_name[NAME_LENGTH];
} Msg;


// Dane serwera
//////////////////////////////
typedef union ConnectionAddress{
    struct sockaddr_un unix_addr;
    struct sockaddr_in ipv4_addr;
} Address;

typedef struct Client{
    Address address;
    int addr_len;
    int socket_fd;
    struct Client* opponent;
    GameStatus* match;
    Player player_info;
    int is_responding;

} Client;

// typedef enum ServerNotificationType{
//     client_noti,
//     socket_noti
// } ServerNotiType;

// typedef struct ServerNotification{
//     ServerNotiType noti_type;
//     int socket_fd;             // Tylko jedno uzywane naraz
//     Client* client;     // Jedno naraz
// } ServerNoti;