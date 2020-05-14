#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <ctype.h>
#include <pthread.h>
#include <math.h>

#define BUFFER_SIZE 128


int** pic_array = NULL;
int** threads_histograms = NULL;
pthread_t* my_threads = NULL;   //Globalne aby w razie bledu dalo sie usunac
int* thread_idx = NULL;         //j.w.

int COLS = -1;
int ROWS = -1;
int GRAY_SCALE = -1;
int THREAD_COUNT = -1;


// Funkcje pomocnicze
////////////////////////////////////////////

void errExit(const char* err_msg, const char* additional_msg, int exit_code){
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

void printBaboon(){
    for(int i=0; i < ROWS; i++){
        for(int j=0; j < COLS; j++)
            printf("%d ", pic_array[i][j]);
        printf("\n");
    }
}

void deallocAll(){
    if (pic_array != NULL){
        for(int i=0; i < ROWS; i++)
            free(pic_array[i]) ;
        free(pic_array);
    }
    if (threads_histograms != NULL){
        for(int i=0; i < THREAD_COUNT; i++)
            free(threads_histograms[i]) ;
        free(threads_histograms);
    }
    if (my_threads != NULL)
        free(my_threads);
    if (thread_idx != NULL)
        free(thread_idx);
}

int stopTimer(struct timeval start_time){
    struct timeval end_time;
    gettimeofday(&end_time, NULL);
    return (end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec);
}

//////////////////////////////////

void saveHistogramToFile(const char* output_file){
    FILE* output_handle = fopen(output_file, "w");
    if (output_handle == NULL)
        errExit("Nie udalo sie stworzyc pilku wyjsciowego o nazwie:", output_file, 8);
    
    fprintf(output_handle, "P2\n");
    fprintf(output_handle, "# Liczba watkow: %d\n",THREAD_COUNT);
    fprintf(output_handle, "%d %d\n", COLS, ROWS);
    fprintf(output_handle, "%d\n", GRAY_SCALE);
    fprintf(output_handle,"#scale: val\n");
    
    for(int i=0; i < GRAY_SCALE; i++){
        int total_sum = 0;
        for(int j=0; j < THREAD_COUNT; j++)
            total_sum += threads_histograms[j][i];
        fprintf(output_handle,"%d: %d\n", i, total_sum);
    }
    fclose(output_handle);
}

void printThreadWorkTime(long int thread, int time_in_micro, int idx, FILE* times_file){
    int seconds = time_in_micro / 1000000;
    int milisec = time_in_micro / 1000;
    int microsec = time_in_micro % 1000;

    printf("Watek %d (%ld): %ds %dms %dus\n", idx, thread, seconds, milisec, microsec);
    fprintf(times_file,"Watek %d (%ld): %ds %dms %dus\n", idx, thread, seconds, milisec, microsec);
}

void printTotalWorkTime(int time_in_micro, FILE* times_file){
    int seconds = time_in_micro / 1000000;
    int milisec = time_in_micro / 1000;
    int microsec = time_in_micro % 1000;

    printf("\nSumaryczny czas: %ds %dms %dus\n\n\n", seconds, milisec, microsec);
    fprintf(times_file,"\nSumaryczny czas: %ds %dms %dus\n\n\n", seconds, milisec, microsec);
}

void saveToTimesFile(const char* times_file_name, pthread_t* my_threads, struct timeval total_start_time, const char* mode){
    FILE* times_file = fopen(times_file_name, "a");
    if (times_file == NULL)
        errExit("Nie udalo sie utworzyc pliku z czasami dzialnia watkow, o nazwie", times_file_name, 9);
    
    fprintf(times_file,"\t Metoda: %s, liczba watkow: %d\n", mode, THREAD_COUNT);
    
    for(int i=0; i < THREAD_COUNT; i++){
        int* workTime = NULL;  
        pthread_join(my_threads[i], (void**) &workTime);
        printThreadWorkTime(my_threads[i], *workTime, i, times_file);
    }
    
    int total_elapsed_time = stopTimer(total_start_time);
    printTotalWorkTime(total_elapsed_time, times_file);
    fprintf(times_file,"------------------------\n");
    fclose(times_file);
}


void skipComments(FILE* input_handle, char* buffer){
    while( fscanf(input_handle, "%s", buffer) != 0 && buffer[0] == '#'){
        fseek(input_handle, -(strlen(buffer) * sizeof(char)), SEEK_CUR);    // Cofamy sie o dlugosc slowa ze znakiem '#'
        fgets(buffer, BUFFER_SIZE, input_handle);       // Wciagamy cala linijke i ja pomijamy
    }
    fseek(input_handle, -(strlen(buffer) * sizeof(char)), SEEK_CUR);        // Cofamy sie aby moc czytac 
}

void * signRoutine(void* arg){
    int my_idx = *((int*) arg);     // arg-> wskaznik na int, aby wyciagnac jego wartosc
    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    int min_gray_scale = my_idx * ceil(1.f * GRAY_SCALE / THREAD_COUNT);
    int max_gray_scale = (my_idx == THREAD_COUNT - 1) ? GRAY_SCALE : (my_idx + 1) * ceil(1.f * GRAY_SCALE / THREAD_COUNT);

    for(int i=0; i < ROWS; i++)
        for(int j=0; j < COLS; j++){
            int pixel = pic_array[i][j];
            if (min_gray_scale <= pixel && pixel < max_gray_scale)
                threads_histograms[my_idx][pixel]++;
        }
    
    int* elapsed_time =(int*) malloc(sizeof(int));
    *elapsed_time = stopTimer(start_time);    
    return elapsed_time;
}

void * blockRoutine(void* arg){
    int my_idx = *((int*) arg);        // arg-> wskaznik na int, aby wyciagnac jego wartosc
    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    int min_col = my_idx * ceil(1.f * COLS / THREAD_COUNT);
    int max_col = (my_idx == THREAD_COUNT -1) ? COLS : (my_idx+1) * ceil(1.f * COLS / THREAD_COUNT);

    for(int i=0; i < ROWS; i++)
        for(int j=min_col; j < max_col; j++){
            int pixel = pic_array[i][j];
            threads_histograms[my_idx][pixel]++;
        }
    
    int* elapsed_time =(int*) malloc(sizeof(int));
    *elapsed_time = stopTimer(start_time);    
    return elapsed_time;
}

void * interleavedRoutine(void* arg){
    int my_idx = *((int*) arg);       // arg-> wskaznik na int, aby wyciagnac jego wartosc
    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    for(int i=0; i < ROWS; i++)
        for(int j=my_idx; j < COLS; j += THREAD_COUNT){
            int pixel = pic_array[i][j];
            threads_histograms[my_idx][pixel]++;
        }
    int* elapsed_time =(int*) malloc(sizeof(int));
    *elapsed_time = stopTimer(start_time);    
    return elapsed_time;
}

void loadImageFromFile(char* filename){
    FILE* input_handle = fopen(filename, "r");
    if (input_handle == NULL)
        errExit("Nie znaleziono podanego pliku:", filename, 10);
    
    char buffer[BUFFER_SIZE];           // Wg tego co jest napisane na stronie to zadna linijka nie powinna byc dluzsza niz 70 znakow

    skipComments(input_handle, buffer);
    if (fscanf(input_handle,"%s", buffer) == EOF)
        errExit("Wystapil problem z odczytaniem typu obrazu wejsciowego!", NULL, 11);
    if ( strcmp(buffer, "P2") != 0 )
        errExit("Podany format obrazu wejsciowego nie jest formatem ASCII PGM!", NULL, 12);
    
    skipComments(input_handle, buffer);
    if (fscanf(input_handle, "%d", &COLS) == EOF)           // Szerokosc
        errExit("Wystapil problem z odczytaniem rozmiaru obrazu wejsciowego!", NULL, 11);      

    skipComments(input_handle, buffer);
    if (fscanf(input_handle, "%d", &ROWS) == EOF)           // Wysokosc
        errExit("Wystapil problem z odczytaniem rozmiaru obrazu wejsciowego!", NULL, 11);

    skipComments(input_handle, buffer);
    if (fscanf(input_handle, "%d", &GRAY_SCALE) == EOF)     // Skala szarosci
        errExit("Wystapil problem z odczytaniem skali szarosci obrazu wejsciowego!", NULL, 11);

    skipComments(input_handle, buffer);

    if (COLS <= 0 || ROWS <= 0 || GRAY_SCALE <= 0)
        errExit("Nie poprawnie odczytano parametry obrazu z pliku!", NULL, 13);

    pic_array = (int**) calloc (ROWS, sizeof(int*));     // Wystarczylo by zamiast intow robic char, ale chyba jest to mniej intuicyjne
    for(int i=0; i < ROWS; i++)
        pic_array[i] = (int*)calloc(COLS, sizeof(int));
    
    
    threads_histograms = (int**) calloc(THREAD_COUNT, sizeof(int*));
    for(int i=0; i < THREAD_COUNT; i++)
        threads_histograms[i] = (int*) calloc(GRAY_SCALE, sizeof(int));

    // Czytanie obrazu
    for (int i=0; i < ROWS; i++){
        for (int j=0; j < COLS; j++){
            skipComments(input_handle,buffer);
            if (fscanf(input_handle, "%d", &pic_array[i][j]) == EOF)   
                errExit("Wystapil problem z odczytaniem 'pixeli' obrazu wejsciowego!", NULL, 11);
        }
    }
    fclose(input_handle);
}

int main(int argc, char** argv){
    if (atexit(deallocAll) != 0)
        errExit("Nie udalo sie zarejestrowac poprawnej funkcji do wywolania na wyjsci!", NULL, 6);

    char needed_args[] = "Potrzebne argumenty: liczba_watkow sign/block/intervaled input_image output_file";

    if (argc != 5)
        errExit("Niepoprawna liczba argumentow!", needed_args, 1);

    if ( !isANumber(argv[1]) || atoi(argv[1]) <= 0)
        errExit("Liczba watkow musi byc dodatnia liczba!", needed_args, 2);
    
    THREAD_COUNT = atoi(argv[1]);
    char* mode = argv[2];
    char* input_file = argv[3];
    char* output_file = argv[4];

    if ( strcmp(mode, "sign") != 0 && strcmp(mode, "block") != 0 && strcmp(mode, "interleaved") != 0)
        errExit("Nieznany tryb dzialania:", mode, 4);

    loadImageFromFile(input_file);
    
    struct timeval total_start_time;
    gettimeofday(&total_start_time, NULL);

    my_threads = (pthread_t*) calloc(THREAD_COUNT, sizeof(pthread_t));
    thread_idx = (int*) calloc(THREAD_COUNT, sizeof(int));
    for(int i=0; i < THREAD_COUNT; i++){
        thread_idx[i] = i;
        if ( strcmp(mode,"sign") == 0)
            pthread_create(&my_threads[i], NULL, &signRoutine, &thread_idx[i]);
        else if ( strcmp(mode,"block") == 0)
            pthread_create(&my_threads[i], NULL, &blockRoutine, &thread_idx[i]);
        else        // interleaved
            pthread_create(&my_threads[i], NULL, &interleavedRoutine, &thread_idx[i]);
    }
    
    saveToTimesFile("Times.txt", my_threads, total_start_time,  mode);
    
    saveHistogramToFile(output_file);
   
    return 0;
}