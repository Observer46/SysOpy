#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <errno.h>
#include <signal.h>

#define UNIX_PATH_MAX 108       // Jest w linux/un.h ale to koliduje z sys/un.h
#define NAME_LENGTH 32
#define COMMUNICATION_TYPE SOCK_STREAM
#define MAX_PENDING_EVENTS 4
#define MAX_SERVER_PENDING_EVENTS 16
#define MAX_PLAYERS_ON_SERVER 4
#define PING_TIMER 10           // Co 10 sekund sprawdzamy czy odpowiadaja

void errExit(const char* err_msg, const char* additional_msg, int exit_code);
int isANumber(char* number);