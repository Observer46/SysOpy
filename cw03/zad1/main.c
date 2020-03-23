#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/times.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define __USE_XOPEN_EXTENDED 1
#include <ftw.h>

void errorExit(){
    exit(1);
}


void find(char* path){
    if (path == NULL)
        return;

    struct stat startingDirStatus;
    if ( lstat(path, &startingDirStatus) != 0 ){     // Jesli nieudalo sie odczytac poczatkowy katalog
        printf("Nie udalo sie odczytac statusu folderu startowego! %s!\n",path);
        errorExit();
    }
    



    pid_t pid = fork();
    if (pid == 0){  // Potomek przeglada
        int thisPID = (int) getpid();
        printf("\n%s, PID: %d\n", path, thisPID);
        int lsStatus = execlp("ls","ls","-l", path, NULL);

        if (lsStatus == -1){        // Tutaj program dojdzie tylko w przypadku bledu wykonania ls -l
            fprintf(stderr,"Nie podwiodlo sie uruchomienie ls -l dla katalogu %s w procesie o PID: %d", path, thisPID);
            errorExit();
        }
    }  
    else{           // Przodek czeka az potomek wykona ls -l, dzieki temu nie uzyskamy losowej kolejnosci wywolan ls -l w roznych folderach
        wait(0);
    }

    DIR* currDir = opendir(path);
    if ( currDir == NULL ){
        fprintf(stderr,"Nie udalo sie odczytac katalogu %s!\n",path);
        //errorExit();    // Zamiast wychodzic kontynujemy wyszukiwanie
        return;
    }

    struct dirent* file = readdir(currDir);
    while( file != NULL){
        char* filename = file -> d_name;
        struct stat fileStatus;

        char filepath[4096];
        strcpy(filepath, path);
        if (filepath[strlen(filepath) - 1]!='/')
            strcat(filepath,"/");
        strcat(filepath,filename);

        if ( lstat(filepath, &fileStatus) == 0 ){  // Jesli udalo sie odczytac status pliku
            if ( strcmp(filename,".") && strcmp(filename,"..")  )
                if ( S_ISDIR (fileStatus.st_mode))
                    find(filepath);  // Wchodzimy glebiej, jesli to folder i nie "." ani ".."
        }
        else{
            printf("Nie udalo sie odczytac statusu pliku %s!\n",filepath);
            errorExit();
        }
        file = readdir(currDir);
    }
    closedir(currDir);
    return;
    
}

static int find_nftw(const char* path, const struct stat* fileStatus, int flags, struct FTW* ftw){
    if ( S_ISDIR(fileStatus -> st_mode) ){
        pid_t relativePID = fork();
        if (relativePID == 0){      // Potomek wywoluje ls
            int thisPID = (int) getpid();
            printf("\n%s, PID: %d\n", path, thisPID);
            int lsStatus = execlp("ls","ls","-l", path, NULL);

            if (lsStatus == -1){        // Tutaj program dojdzie tylko w przypadku bledu wykonania ls -l
                fprintf(stderr,"Nie podwiodlo sie uruchomienie ls -l dla katalogu %s w procesie o PID: %d", path, thisPID);
                errorExit();
            }
        }
        else{
            wait(0);
        }
    }
    return 0;

}


int main(int argc, char** argv){
    int isNFTW = 0;    // Tryb dzialnia nftw = 1, lib = 0, -1 = nieokreslone

    char pathBuffor[4096];
    strcpy(pathBuffor,"");  // "Zerujemy" sciezke

    for (int i=1; i < argc; i++){
        char* cmd = argv[i];
        if ( strcmp(cmd,"-nftw") == 0 )
            isNFTW = 1;
        else{
            if( strcmp(pathBuffor,"") ){
                fprintf(stderr,"Juz podano sciezke do wyszukiwania!\n");
                errorExit();
            }
            strcpy(pathBuffor,argv[i]);
        }
    }
    

    if (strcmp(pathBuffor,"") == 0)
        strcpy(pathBuffor,".");     // Domyslna wartosc - obecny katalog 

    if (isNFTW)
        nftw(pathBuffor, &find_nftw, 16, FTW_PHYS);
    else
        find(pathBuffor);
    
    return 0;
}