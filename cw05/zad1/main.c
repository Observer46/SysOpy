#include <stdio.h>
#include <stdlib.h>

void err_exit_limits(){
    fprintf(stderr,"Osiagnieto maksymalny limit ilosci komend lub argumentow do pojedynczej komendy!\n");
    exit(10);
}

int parse_command_line(FILE* command_file, char*** command_strings, int line_length){       // zwraca faktyczna ilosc komend, 
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
            if (command_count >= 256)
                err_exit_limits();
            arg_counter = 0;
        }
        
        char cmd_arg[256];
        strcpy(cmd_arg, command_part);

        if ( cmd_arg[strlen(cmd_arg) - 1] == '\n' ){
            newline_found = 1;
            cmd_arg[strlen(cmd_arg) - 1] = '\0';
        }
        command_strings[command_count][arg_counter] = cmd_arg;
        arg_counter++;

        if (arg_counter >= 256)
            err_exit_limits();
    }

    command_strings[command_count][arg_counter] = NULL;         // Ostatni NULL
    return command_count;
}

int count_line_length(FILE* command_file){
    int line_length = 0;
    char chr = getc();
    while (chr != EOF && chr !+ '\n'){
        line_length++;
        chr = getc();
    }
    fseek(command_file, -line_length, SEEK_CUR);             // Wracamy na poczatek linii
    return line_length;
}

int main(int argc, char** argv){
    const int maxCommands = 256;
    const int maxArgsPerCommand = 256;

    if (argc != 2){
        fprintf(stderr,"Prosze podac dokladnie 1 argument!\nArgument ten powinien byc sciezka do pliku z ktorego beda interpretowane polecenia\n");
        exit(1);
    }

    char* filepath = argv[1];

    FILE* command_file = fopen(filepath, "r");
    if (command_file == NULL){
        fprintf(stderr,"Nie udalo sie otworzyc pliku: %s\n",filepath);
        exit(2);
    }

    int pipe_array[maxCommands][2];
    char* command_strings[maxCommands][maxArgsPerCommand];

    while( !feof(command_file) ){        // Do sprawdzenia
        int line_length = count_line_length(command_file);

        char* command_line = (char*)calloc(line_length, sizeof(char));
        int command_count = parse_command_line(command_file);

        // if ( fgets(command_line, line_length, command_file) == NULL ){
        //     free(command_line);
        //     fclose(command_file);
        //     fprintf(stderr,"Nie udalo sie odczytac linii komendy z pliku wejsciowego!\n");
        //     exit(3);
        // }


        for (int i=0; i < command_count; i++){  // Proces macierzysty nie uzywa potokow, wiec zamyka obie koncowki
            fclose(pipe_array[i][0]);
            fclose(pipe_array[i][1]);
        }
        
        for (int i=0; i < command_count; i++)   ,// Proces macierzysty czeka na wszystkich potomkow
            wait(NULL);
        free(command_line)
    }

    

    fclose(command_file);



    return 0;
}