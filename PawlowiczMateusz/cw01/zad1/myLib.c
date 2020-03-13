#include "myLib.h"

struct BlockArray* createBlockArray(int size){
    if(size == 0){
        printf("Nie mozna stworzy tablicy blokow o rozmiarze 0!\n");
        exit(1);
    }
    struct BlockArray* array = (struct BlockArray*) calloc(1,sizeof(struct BlockArray));
    array -> blocks = (struct OpBlock**) calloc(size,sizeof(struct OpBlock));
    array -> size = size;
    array -> iter = 0;      // To wskazuje na pierwszy wolny index
    return array;
};

struct OpBlock* createOpBlock(int size){
    struct OpBlock* opBlock = (struct OpBlock*)calloc(1,sizeof(struct OpBlock));
    if(size == 0)
        opBlock -> operations = NULL;
    else
        opBlock -> operations = (char**) calloc(size,sizeof(char*));
    opBlock -> opCount = size;
    return opBlock;
};

int parseOperationsFromTmp(struct OpBlock* opBlock, char* tmpfileName){
    FILE* tmpFile = fopen(tmpfileName,"r");
    if(tmpFile == NULL){
        printf("Nie udalo sie otworzyc pliku tymczasowego - blad programu!");
        exit(1);
    }

    char buffer[400];       //Bufor na linijke 
    int operIdx = -1;
    char operation[10000]; //Max. bufor na operacje        fseek itp. do ogarniecia
    strcpy(operation, "");  //Pusty

    while(!feof(tmpFile)){
        fgets(buffer,400,tmpFile);
        if( isdigit(buffer[0]) ){
            
            if(operIdx >= 0){
                opBlock -> operations[operIdx] = (char*) calloc(strlen(operation) + 10, sizeof(char));   // +10 na zapas
                strcpy(opBlock -> operations[operIdx], operation);
                strcpy(operation,"");
            }
            operIdx++;
        }
        strcat(operation,buffer);

    }
    if (opBlock -> operations != NULL){     //Dodajemy ostatnie, o ile wgl sa jakies operacje do dodania
        opBlock -> operations[operIdx] = (char*) calloc(strlen(operation), sizeof(char));
        strcpy(opBlock -> operations[operIdx], operation);
    }

    fclose(tmpFile);
    return opBlock -> opCount;
}

int compareTwoFiles(struct BlockArray* blockArray, const char* file1,const char* file2, int idx){

    FILE* fstFile = fopen(file1,"r");
    if (fstFile == NULL){
        printf("Plik %s podany w sekwencji nie istnieje!\n",file1);
        exit(1);
    }
    fclose(fstFile);

    FILE* sndFile = fopen(file2,"r");
    if (sndFile == NULL){
        printf("Plik %s podany w sekwencji nie istnieje!\n",file2);
        exit(1);
    }
    fclose(sndFile);

    int opCount = 0;

    char cmdbuffor [100];       
    sprintf(cmdbuffor,"diff %s %s >tmp.txt",file1, file2);
    system(cmdbuffor);
    FILE* tmpFile = fopen("tmp.txt","r");
    
    
    
    if(tmpFile == NULL){
        printf("Nie udalo sie otworzyc pliku tymczasowego - blad programu!");
        exit(1);
    }
    //Czytamy plik tymczasowy
    char* buffer = (char*)calloc(400,sizeof(char));
    

    while(!feof(tmpFile)){
        fgets(buffer, 400, tmpFile);
        if( isdigit(buffer[0]) ) opCount++;  //Kazda linijka zaczynajca sie od cyfry to nowa operacja
    }
    fclose(tmpFile);
    struct OpBlock* opBlock = createOpBlock(opCount);
    
    parseOperationsFromTmp(opBlock, "tmp.txt");
    blockArray -> blocks[idx] = opBlock;
    return idx; 
};



void parseFilePairs (struct BlockArray* blockArray ,char** filePairsList, int pairsNumber){     //filePairsList.size = 2*pairsNumber
    if (blockArray == NULL){
        printf("Nie stworzono tablicy blokow, wiec nie mozna wprowadzic sekwencji par plikow!\n");
        for(int i=0; i < 2*pairsNumber; i++)
            free(filePairsList[i]);
        free(filePairsList);
        exit(1);
    }
    
    if( (blockArray -> size) - (blockArray -> iter) < pairsNumber){               // Po lewej stronie nierownosci aktualna ilosc wolnych miejsc
        printf("Zbyt dluga sekwencja plikow dla takiej tablicy\n");
        exit(1);
    }

    
    for(int i=0; i < pairsNumber; i++){
        const char* file1 = filePairsList[2*i];
        const char* file2 = filePairsList[2*i+1];
        int idx = blockArray -> iter;
        compareTwoFiles(blockArray, file1, file2, idx);
        blockArray -> iter++;
    }

    char* tmpfileName = "tmp.txt";
    int del_status = remove(tmpfileName);
    if(del_status != 0){
        printf("Nie udalo sie usunac pliku tymczasowego");
        exit(1);
    }
}


// wyniki mozna traktowac jak kody bledow

int deleteOperation(struct BlockArray* blockArray, int blockIdx, int operationIdx){
    if(blockArray == NULL || blockIdx >= blockArray -> size ||  blockArray -> blocks[blockIdx] == NULL){
        printf("Nie mozna usunac z bloku %d operacji %d, bo nie ma takiego bloku\n!",blockIdx, operationIdx);
        return 1;
    }

    if(blockArray -> blocks[blockIdx] -> operations == NULL){
        printf("Nie mozna usunac z bloku %d operacji %d, bo nigdy jej nie bylo!\n",blockIdx, operationIdx);
        return 2;
    }

    if(operationIdx >= blockArray -> blocks[blockIdx] -> opCount){
        printf("W bloku %d nigdy nie bylo operacji o numerze %d!\n",blockIdx,operationIdx);
        return 3;
    }

    char* operationToDelete = blockArray -> blocks[blockIdx] -> operations[operationIdx];
    if(operationToDelete != NULL){
        free(operationToDelete);
        blockArray -> blocks[blockIdx] -> operations[operationIdx] = NULL;
        return 0;    //Udalo sie usunac 
    }
    printf("Z bloku %d operacja %d juz zostala usunieta!\n",blockIdx, operationIdx);
    return 4;    //Da sie wykorzystac do przechwytywania wyjatkow
}

int deleteOpBlock(struct BlockArray* blockArray, int blockIdx, int printMode){       //printMode: 0 -> nie wypisuje wiadomosci, 1 -> wypisuje
    if(blockArray == NULL){
        if(printMode)
            printf("Nie podano tablicy blokow! (NULL)\n");
        return 1;
    }
    

    if(blockIdx >= blockArray -> size){
        if(printMode)
            printf("Proba usuniecia bloku %d, ktorego nie stworzono! Istnieja bloki: 0-%d\n.",blockIdx,blockArray -> size - 1);
        return 2;
    }

    struct OpBlock* blockToDelete = blockArray -> blocks[blockIdx];
    if(blockToDelete != NULL){
        
        if (blockToDelete -> operations != NULL){
            
            for(int i=0; i < blockToDelete -> opCount; i++){
                char* operationToDelete = blockToDelete -> operations[i];

                if (operationToDelete != NULL)  {
                    free(operationToDelete);
                    blockToDelete -> operations[i] = NULL;
                }
            }
        }
        
        free(blockToDelete);
        blockArray -> blocks[blockIdx] = NULL;
        return 0;
    }
    
    if(printMode)
        printf("Blok %d juz zostal usuniety!\n",blockIdx);
    return 3;
}

int deleteBlockArray(struct BlockArray* blockArray){    //Koncowa dealokacja, bez wiadomosci
    if(blockArray != NULL){
        for(int i=0; i < blockArray -> size; i++){
            deleteOpBlock(blockArray, i, 0);
        }
        free(blockArray);
        return 0;
    }
    return 1;
}

void printOperation(struct BlockArray* blockArray, int blockIdx, int operationIdx, int printMode){     //printMode: 0 -> nie wypisuje wiadomosci o bledach, 1 -> robi to
    if(blockArray == NULL || blockIdx >= blockArray -> size || blockArray -> blocks[blockIdx] == NULL || blockArray -> blocks[blockIdx] -> operations == NULL){
        if (printMode)
            printf("W bloku %d brak operacji %d - brak bloku, jest pusty lub nigdy nie stworzono go!\n",blockIdx,operationIdx);
        return;
    }

    if(operationIdx >= blockArray -> blocks[blockIdx] -> opCount){
        if (printMode)
            printf("W bloku %d nigdy nie bylo operacji o numerze %d!\n",blockIdx,operationIdx);
        return;
    }

    char* operation = blockArray -> blocks[blockIdx] -> operations[operationIdx];
    if(operation == NULL){
        if (printMode)
            printf("Z bloku %d usunieto operacje %d!\n",blockIdx,operationIdx);
        return;
    }

    printf("%s",operation);
}
void printBlock(struct BlockArray* blockArray, int blockIdx){
    if(blockArray == NULL || blockIdx >= blockArray -> size || blockArray -> blocks[blockIdx] == NULL){
        printf("Brak bloku o numerze %d do wypisania!\n",blockIdx);
        return;
    }

    if(blockArray -> blocks[blockIdx] -> operations == NULL){
        printf("Blok %d jest pusty!\n",blockIdx);
        return;
    }
    for(int i=0; i < blockArray -> blocks[blockIdx]->opCount; i++){
        printOperation(blockArray, blockIdx, i, 0);
    }
}