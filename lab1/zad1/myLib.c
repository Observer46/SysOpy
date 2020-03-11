#include "myLib.h"

struct BlockArray* createBlockArray(int size){
    struct BlockArray* array = (struct BlockArray*) calloc(1,sizeof(struct BlockArray));
    array -> blocks = (struct OpBlock**) calloc(size,sizeof(struct OpBlock));
    array -> size = size;
    return array;
};

struct OpBlock* createOpBlock(int size){
    struct OpBlock* opBlock = (struct OpBlock*)calloc(1,sizeof(struct OpBlock));
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

    char buffer[400];
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

    opBlock -> operations[operIdx] = (char*) calloc(strlen(operation), sizeof(char));
    strcpy(opBlock -> operations[operIdx], operation);

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
}


//traktuejmy jak kody bledow

int deleteOperation(struct BlockArray* blockArray, int blockIdx, int operationIdx){
    if(blockArray == NULL ||  blockArray -> blocks[blockIdx] == NULL)
        return 1;

    char* operationToDelete = blockArray -> blocks[blockIdx] -> operations[operationIdx];
    if(operationToDelete != NULL){
        free(operationToDelete);
        return 0;    //Udalo sie usunac 
    }

    return 2;    //Da sie wykorzystac do przechwytywania wyjatkow
}

int deleteOpBlock(struct BlockArray* blockArray, int blockIdx){
    if(blockArray == NULL)  return 1;

    struct OpBlock* blockToDelete = blockArray -> blocks[blockIdx];
    if(blockToDelete != NULL){
        for(int i=0; i < blockToDelete -> opCount;i++)
            deleteOperation(blockArray, blockIdx, i);
        free(blockToDelete);
        return 0;
    }

    return 1;
}

int deleteBlockArray(struct BlockArray* blockArray){
    if(blockArray != NULL){
        for(int i=0; i < blockArray -> size; i++)
            deleteOpBlock(blockArray, i);
        return 0;
    }

    return 1;
}

void printOperation(struct BlockArray* blockArray, int blockIdx, int operationIdx){
    if(blockArray == NULL)  return;
    if(blockArray -> blocks[blockIdx] == NULL)  return;
    char* operation = blockArray -> blocks[blockIdx] -> operations[operationIdx];
    if(operation == NULL)   return;
    printf("%s",operation);
}
void printBlock(struct BlockArray* blockArray, int blockIdx){
    if(blockArray == NULL)  return;
    if(blockArray -> blocks[blockIdx] == NULL)  return;
    printf("\n%d\n",blockArray -> blocks[blockIdx]->opCount);
    for(int i=0; i < blockArray -> blocks[blockIdx]->opCount; i++){
        printOperation(blockArray, blockIdx, i);
    }
}