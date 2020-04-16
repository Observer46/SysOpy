#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

// Limity dla ilosci komend i argumentow do nich
const int maxCommands = 256;
const int maxArgsPerCommand = 256;

void free_command_strings(char* command_strings[maxCommands][maxArgsPerCommand]){
    for (int i=0; i < maxCommands; i++)
        for (int j=0; j < maxArgsPerCommand; j++)
            if (command_strings[i][j] != NULL)
                free(command_strings[i][j]);  
}

void err_exit_limits(char* command_strings[maxCommands][maxArgsPerCommand]){
    fprintf(stderr,"Osiagnieto maksymalny limit ilosci komend lub argumentow do pojedynczej komendy!\n");
    free_command_strings(command_strings);
    exit(10);
}

int parse_command_line(FILE* command_file, char* command_strings[maxCommands][maxArgsPerCommand], int line_length){       // zwraca faktyczna ilosc komend, 
                                                                // commandFile to wskaznik na aktualna pozycje w pliku
    char command_part[256] = "";  // Zakladam, ze pojedynczy kawalek komendy (nazwa badz tez argumenty) nie maja wiecej niz 256 znakow

    int command_count = 0;
    int arg_counter = 0;
    int newline_found = 0;
    
    while( !newline_found ){
        fscanf(command_file,"%s",command_part);
        if ( strcmp(command_part,"|") == 0 ){
            command_strings[command_count][arg_counter] = NULL;
            command_count++;
            if (command_count >= maxCommands)
                err_exit_limits(command_strings);
            arg_counter = 0;
            continue;
        }

        if (command_strings[command_count][arg_counter] == NULL){
            char* cmd_arg = (char*)calloc(256, sizeof(char));
            strcpy(cmd_arg, command_part);
            command_strings[command_count][arg_counter] = cmd_arg;
        }
        else{
            strcpy(command_strings[command_count][arg_counter], command_part);
        }

        char next_char = getc(command_file);

        if ( next_char == '\n' || next_char == EOF){
            newline_found = 1;
            if (++arg_counter >= maxArgsPerCommand)
                err_exit_limits(command_strings);

            command_strings[command_count][arg_counter] = NULL; // Ostanti NULL
            command_count++;
        }
        
        arg_counter++;

        if (arg_counter >= maxArgsPerCommand)
            err_exit_limits(command_strings);    
    }

    return command_count;
}

int count_line_length(FILE* command_file, char* cmd){
    int line_length = 1;
    char chr = getc(command_file);
    while (chr != EOF && chr != '\n'){
        cmd[line_length-1] = chr;
        line_length++;
        chr = getc(command_file);
    }
    cmd[line_length-1] = '\0';
    fseek(command_file, -line_length, SEEK_CUR);             // Wracamy na poczatek linii
    return line_length;
}

int main(int argc, char** argv){
    if (argc != 2){
        fprintf(stderr,"Prosze podac dokladnie 1 - sciezke do pliku z komendami\n");
        exit(1);
    }

    char* filepath = argv[1];

    FILE* command_file = fopen(filepath, "r");
    if (command_file == NULL){
        fprintf(stderr,"Nie udalo sie otworzyc pliku: %s\n",filepath);
        exit(2);
    }

    // Podajac potokami informacje wystarcza nam tylko dwa jednoczesnie: ... | program_i | ... 
    int pipefd[2];
    int prev_pipefd[2];

    char* command_strings[maxCommands][maxArgsPerCommand];
    for (int i=0; i < maxCommands; i++)
        for (int j=0; j < maxArgsPerCommand; j++)
            command_strings[i][j] = NULL;       // Pomaga potem w zwalnaniu pamieci

    char cmd[2048];     // Do wypisywania uzytej komendy

    while( !feof(command_file) ){        // Petla dziala dla kazdej kolejnej linijki

        int line_length = count_line_length(command_file, cmd);
        printf("%s\n",cmd);
        int command_count = parse_command_line(command_file, command_strings, line_length);

        for(int i=0; i < command_count; i++){
            
            if (i < command_count - 1)      // Otwieramy potok przed kazdym programem z wyjatkiem ostatniego
                if (pipe(pipefd) == -1){
                    fprintf(stderr,"Wywolanie pipe() sie nie powiodlo!\n");
                    free_command_strings(command_strings);
                    exit(3);
                }

            pid_t child_pid = fork();
            if (child_pid == -1){
                fprintf(stderr,"Wywolanie fork() sie nie powiodlo!\n");
                free_command_strings(command_strings);
                exit(4);
            }

            if (child_pid == 0){
                
                if (i-1 >= 0){
                    dup2(prev_pipefd[0], STDIN_FILENO); // Podmieniamy standardowe wejscie na poprzedni potok
                    close(prev_pipefd[1]);      // Nie bedzie tutaj nic pisac do poprzedniego potoku, tylko czytac
                }


                if (i < command_count - 1){
                    dup2(pipefd[1], STDOUT_FILENO);     // Podmieniamy standardowe wyjscie na nowy potok
                    close(pipefd[0]);        // Nie bedzie stad nic czytac z nowego potoku, tylko pisac
                }

                execvp(command_strings[i][0],command_strings[i]);
                fprintf(stderr,"Wywolanie execvp(%s) sie nie powiodlo!\n", command_strings[i][0]);
                free_command_strings(command_strings);
                exit(5);
                
            }
            else{
                if (i-1 >= 0){               // Zamykamy tylko te, ktore sa nieuzywane (zamykanie wczesniej spowoduje, ze potomek dostanie zamkniety potok)
                    close(prev_pipefd[0]);      // Proces macierzysty nie uzywa potokow, wiec zamyka obie koncowki
                    close(prev_pipefd[1]);      // Ale zamykamy tylko te, ktore sa juz nie uzywane    
                }
                prev_pipefd[0] = pipefd[0];
                prev_pipefd[1] = pipefd[1];
            }
        }

        
        for (int i=0; i < command_count; i++)   // Proces macierzysty czeka na wszystkich potomkow
            wait(NULL);
        
        printf("\n");
    }
    
    free_command_strings(command_strings);
    fclose(command_file);
    return 0;
}