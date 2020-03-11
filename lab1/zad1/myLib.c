#include "myLib.h"

struct BlockArray* createBlockArray(int size){
    if(size == 0){
        printf("Nie mozna stworzy tablicy blokow o rozmiarze 0!\n");
        exit(1);
    }
    struct BlockArray* array = (struct BlockArray*) calloc(1,sizeof(struct BlockArray));
    array -> blocks = (struct OpBlock**) calloc(size,sizeof(struct OpBlock));
    array -> size = size;
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
    char operation[100000]; //Max. bufor na operacje
    strcpy(operation, "");  //Pusty

    while(!feof(tmpFile)){
        fgets(buffer,400,tmpFile);
        if( isdigit(buffer[0]) ){
            
            if(operIdx >= 0){
                opBlock -> operations[operIdx] = (char*) calloc(strlen(operation), sizeof(char));
                strcpy(opBlock -> operations[operIdx], operation);
                strcpy(operation,"");
            }
            operIdx++;
        }
        strcat(operation,buffer);

        //printf("I'M A PRINTY BOI\n");
    }
    if (opBlock -> operations != NULL){     //Dodajemy ostatnie, o ile wgl sa jakies operacje do dodania
        opBlock -> operations[operIdx] = (char*) calloc(strlen(operation), sizeof(char));
        strcpy(opBlock -> operations[operIdx], operation);
    }

    fclose(tmpFile);
    return opBlock -> opCount;
}

int compareTwoFiles(struct BlockArray* blockArray, const char* file1,const char* file2, int idx){
    int opCount = 0;

    char cmdbuffor [100];       //na razie troche brzydko
    sprintf(cmdbuffor,"diff %s %s >tmp.txt",file1, file2);
    system(cmdbuffor);
    FILE* tmpFile = fopen("tmp.txt","r");
    
    if(tmpFile == NULL){
        printf("Nie udalo sie otworzyc pliku tymczasowego - blad programu!");
        exit(1);
    }
    //Czytamy plik tymczasowy
    char buffer[400];
    

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
    if(blockArray -> size < pairsNumber){               //Do rozwazenia realokacja
        printf("Zbyt dluga sekwencja plikow dla takiej tablicy\n");
        exit(1);
    }


    for(int i=0; i<pairsNumber; i++){
        const char* file1 = filePairsList[i];
        const char* file2 = filePairsList[i+1];
        compareTwoFiles(blockArray, file1, file2, i);
    }
    char* tmpfileName = "tmp.txt";
    int del_status = remove(tmpfileName);
    if(del_status != 0){
        printf("Nie udalo sie usunac pliku tymczasowego");
        exit(1);
    }
}


//traktuejmy jak kody bledow

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
        printf("Pomyslne usuwanie z bloku %d operacji %d.\n", blockIdx, operationIdx);
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
            for(int i=0; i < blockToDelete -> opCount;i++){
                char* operationToDelete = blockToDelete -> operations[i];
                if (operationToDelete != NULL)  free(operationToDelete);
            }
        }
        
        free(blockToDelete);
        if(printMode)
            printf("Pomyslne usuwanie bloku %d.\n",blockIdx);
        return 0;
    }
    if(printMode)
        printf("Blok %d juz zostal usuniety!\n",blockIdx);
    return 3;
}

int deleteBlockArray(struct BlockArray* blockArray){    //Koncowa dealokacja, bez wiadomosci
    if(blockArray != NULL){

        for(int i=0; i < blockArray -> size; i++)
            deleteOpBlock(blockArray, i, 0);
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