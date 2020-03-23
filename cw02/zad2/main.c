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

#define __USE_XOPEN_EXTENDED 1
#include <ftw.h>

int *atimeGlobal = NULL;
int *mtimeGlobal = NULL;
int *maxdepthGlobal = NULL;
char aSignGlobal;
char mSignGlobal;

int isANumber(char* number, char* sign){
    int i=0;
    if ( number[0]=='-' || number[0]=='+'){
        i++;
        *sign = number[0];
        if (strlen(number) == 1)    return 0;
    }
    
    for(; i < strlen(number); i++){
        
        if ( !isdigit(number[i]) )
            return 0;
    }
    return 1;
}


void errorExit(int* atime, int* mtime, int* maxdepth){
    if (atime != NULL)
        free(atime);
    if (mtime != NULL)
        free(mtime);
    free(maxdepth);
    exit(1);
}




int timeCheck(time_t timeToCheck, int* dayCount, char* sign){
    if (dayCount == NULL)   return 1;
    time_t uniCurrTime;
    time(&uniCurrTime);
    struct tm* timeInfo = localtime(&uniCurrTime);
    time_t currTime = mktime(timeInfo);

    int timeDiff = difftime(currTime, timeToCheck) / 86400;     // w sekundach, wiec dzielimy przez 24*60*60 by miec w dniach
    if (*sign == '=' && timeDiff == *dayCount)       return 1;
    else if (*sign == '-' && timeDiff < -*dayCount)   return 1;
    else if (*sign == '+' && timeDiff > *dayCount)     return 1;

    return 0;
}

void printIfMeetsConditions(const char* filepath, struct stat fileStatus, int* atime, int* mtime, char* aSign, char* mSign){   
    time_t accessTime = fileStatus.st_atime;
    time_t modTime = fileStatus.st_mtime;

    if( timeCheck(accessTime, atime, aSign) && timeCheck(modTime, mtime, mSign) ){
        nlink_t linkCount = fileStatus.st_nlink;
        char fileType[15];
        off_t fileSize = fileStatus.st_size;

        char atimeString[32];
        char mtimeString[32];

        strftime(atimeString, 32, "%d-%m-%Y %H:%M:%S", localtime(&accessTime));
        strftime(mtimeString, 32, "%d-%m-%Y %H:%M:%S", localtime(&modTime));


        mode_t type = fileStatus.st_mode;
        if ( S_ISREG(type) )    strcpy(fileType,"file");
        else if ( S_ISDIR(type) )    strcpy(fileType,"dir");
        else if ( S_ISCHR(type) )    strcpy(fileType,"char dev");
        else if ( S_ISBLK(type) )    strcpy(fileType,"block dev");
        else if ( S_ISFIFO(type) )    strcpy(fileType,"fifo");
        else if ( S_ISLNK(type) )    strcpy(fileType,"slink");
        else if ( S_ISSOCK(type) )    strcpy(fileType,"socket");
        else    strcpy(fileType,"unknown");

        printf("%s ----- links: %ld | type: %s | size: %ld | atime: %s | mtime: %s\n",filepath, linkCount, fileType, fileSize, atimeString, mtimeString);
    }
}

void find(char* path, int depth, int* atime, int* mtime, int* maxdepth, char* aSign, char* mSign){
    if (depth >= *maxdepth && *maxdepth >=0)  return;   
    if (depth == 0){        // Wypisywanie sciezki startowej
        struct stat startingDirStatus;
        if ( lstat(path, &startingDirStatus) == 0 )     // Jesli udalo sie odczytac plik
            printIfMeetsConditions(path, startingDirStatus, atime, mtime, aSign, mSign);
        else{
            printf("Nie udalo sie odczytac statusu folderu startowego! %s!\n",path);
            errorExit(atime, mtime, maxdepth);
        }
    }

    DIR* currDir = opendir(path);
    if ( currDir == NULL ){
        fprintf(stderr,"Nie udalo sie odczytac katalogu %s!\n",path);
        //errorExit(atime, mtime, maxdepth);    // Zamiast wychodzic kontynujemy wyszukiwanie
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
            if ( strcmp(filename,".") && strcmp(filename,"..")  ){
                printIfMeetsConditions(filepath, fileStatus, atime,  mtime, aSign, mSign);    // Wypisujemy wszystkie poza "." i ".."

                if ( S_ISDIR (fileStatus.st_mode))
                    find(filepath, depth + 1, atime, mtime, maxdepth, aSign, mSign);  // Wchodzimy glebiej, jesli to folder i nie "." ani ".."
            }
        }
        else{
            printf("Nie udalo sie odczytac statusu pliku %s!\n",filepath);
            errorExit(atime, mtime, maxdepth);
        }

        file = readdir(currDir);
    }

    closedir(currDir);
}


void printIfMeetsConditions_ntfw(const char* filepath, const struct stat* fileStatus, int* atime, int* mtime, char* aSign, char* mSign){   
    time_t accessTime = fileStatus -> st_atime;
    time_t modTime = fileStatus -> st_mtime;

    if( timeCheck(accessTime, atime, aSign) && timeCheck(modTime, mtime, mSign) ){
        nlink_t linkCount = fileStatus -> st_nlink;
        char fileType[15];
        off_t fileSize = fileStatus -> st_size;

        char atimeString[32];
        char mtimeString[32];

        strftime(atimeString, 32, "%d-%m-%Y %H:%M:%S", localtime(&accessTime));
        strftime(mtimeString, 32, "%d-%m-%Y %H:%M:%S", localtime(&modTime));


        mode_t type = fileStatus -> st_mode;
        if ( S_ISREG(type) )    strcpy(fileType,"file");
        else if ( S_ISDIR(type) )    strcpy(fileType,"dir");
        else if ( S_ISCHR(type) )    strcpy(fileType,"char dev");
        else if ( S_ISBLK(type) )    strcpy(fileType,"block dev");
        else if ( S_ISFIFO(type) )    strcpy(fileType,"fifo");
        else if ( S_ISLNK(type) )    strcpy(fileType,"slink");
        else if ( S_ISSOCK(type) )    strcpy(fileType,"socket");
        else    strcpy(fileType,"unknown");

        printf("%s ----- links: %ld | type: %s | size: %ld | atime: %s | mtime: %s\n",filepath, linkCount, fileType, fileSize, atimeString, mtimeString);
    }
}

static int find_nftw(const char* path, const struct stat* fileStatus, int flags, struct FTW* ftw){
    if ( ftw -> level > *maxdepthGlobal && *maxdepthGlobal >= 0)   return 0;
    printIfMeetsConditions_ntfw(path, fileStatus, atimeGlobal, mtimeGlobal, &aSignGlobal, &mSignGlobal);
    return 0;

}


int main(int argc, char** argv){
    int* atime = NULL;
    int* mtime = NULL;
    int* maxdepth = (int*)calloc(1,sizeof(int));
    char aSign = '=';
    char mSign = '=';
    *maxdepth = -1;     // Brak limitu glebokosci szukania
    
    int isNFTW = 0;    // Tryb dzialnia nftw = 1, lib = 0, -1 = nieokreslone

    char pathBuffor[4096];
    strcpy(pathBuffor,"");  // "Zerujemy" sciezke

    for (int i=1; i < argc; i++){
        char* cmd = argv[i];

        if ( strcmp(cmd,"-atime") == 0){
            if (atime != NULL){
                fprintf(stderr, "Juz okreslono wartosc parametru dla -atime!\n");
                errorExit(atime, mtime, maxdepth);
            }

            if (i+1 >=argc || !isANumber(argv[i+1], &aSign)){
                fprintf(stderr, "Podano bledna wartosc parametru dla -atime (musi to byc liczba calkowita)\n");
                errorExit(atime, mtime, maxdepth);
            }
            i++;
            atime = (int*)calloc(1,sizeof(int));
            *atime = atoi(argv[i]);
        }
        else if ( strcmp(cmd,"-mtime") == 0){
            if (mtime != NULL){
                fprintf(stderr, "Juz okreslono wartosc parametru dla -mtime!\n");
                errorExit(atime, mtime, maxdepth);
            }

            if (i+1 >=argc || !isANumber(argv[i+1], &mSign)){
                fprintf(stderr, "Podano bledna wartosc parametru dla -mtime (musi to byc liczba calkowita)\n");
                errorExit(atime, mtime, maxdepth);
            }
            i++;
            mtime = (int*)calloc(1,sizeof(int));
            *mtime = atoi(argv[i]);
        }
        else if ( strcmp(cmd,"-maxdepth") == 0){
            if (*maxdepth >= 0){
                fprintf(stderr, "Juz okreslono wartosc parametru dla -maxdepth\n");
                errorExit(atime, mtime, maxdepth);
            }
            
            char maxdepthSign = '=';
            if (i+1 >=argc || !isANumber(argv[i+1], &maxdepthSign)){
                fprintf(stderr, "Podano bledna wartosc parametru dla -atime (musi to byc liczba calkowita)\n");
                errorExit(atime, mtime, maxdepth);
            }
            if (maxdepthSign == '-'){
                fprintf(stderr, "Opcja -maxdepth musi miec nieujemna wartosc!\n");
                errorExit(atime, mtime, maxdepth);
            }
            i++;
            *maxdepth = atoi(argv[i]);
        }
        else if ( strcmp(cmd,"-nftw") == 0 )
            isNFTW = 1;
        else{
            if( strcmp(pathBuffor,"") ){
                fprintf(stderr,"Juz podano sciezke do wyszukiwania!\n");
                errorExit(atime, mtime, maxdepth);
            }

            strcpy(pathBuffor,argv[i]);
        }
    }
    

    if (strcmp(pathBuffor,"") == 0)
        strcpy(pathBuffor,".");     // Domyslna wartosc - obecny katalog 

    if (isNFTW){
        atimeGlobal = atime;
        mtimeGlobal = mtime;
        maxdepthGlobal = maxdepth;
        aSignGlobal = aSign;
        mSignGlobal = mSign;
        nftw(pathBuffor, &find_nftw, 16, FTW_PHYS);
    }
    else
        find(pathBuffor, 0, atime, mtime, maxdepth, &aSign, &mSign);
    
    free(atime);
    free(mtime);
    free(maxdepth);
    return 0;
}