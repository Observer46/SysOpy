#include "utils.h"

void errExit(const char* err_msg, const char* additional_msg, int exit_code){
    printf("Errno: %s\n", strerror(errno));
    fprintf(stderr, "%s\n", err_msg);
    if (additional_msg != NULL)
        printf("%s\n", additional_msg);
    exit(exit_code);
}

int isANumber(char* number){
    for(int i=0; i < strlen(number); i++)
        if ( !isdigit(number[i]) )
            return 0;
    return 1;
}