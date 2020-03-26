#include "matrixMethods.h"

// Zestaw metod dostepowych do plikow z macierzami

// UWAGA: Przyjeta konwecja - na kazda liczbe w pliku przeznaczone jest 10 znakow - daje teortycznie liniowy dostep
// Zalozenie - zeby ograniczyc ilosc i/o to w kazdej funkcji plik 'matrix' jest juz otwarty

const int size_of_element(){
    return 10;
}

int isANumber(char* number){        // Akceptuje tylko dodatnie liczby 
    for(int i=0; i < strlen(number); i++)
        if ( !isdigit(number[i]) )
            return 0;
    return 1;
}

int get_row_length(FILE* matrix){
    fseek(matrix, 0, 0);
    char chr = getc(matrix);
    int rowLength = 1;

    while(chr != '\n' && chr != EOF){
        chr = getc(matrix);
        rowLength++;
    }

    if (chr == EOF){
        fprintf(stderr,"Proba policzenia ilosci kolumn pustego pliku!\n");
        exit(1);
    }

    fseek(matrix, 0, 0);
    return rowLength;
}

int count_cols(FILE* matrix){
    fseek(matrix, 0, 0);        // Liczymy kolumny w pierwszym wierszu
    char chr = getc(matrix);
    int colCount = 0;
    int colFlag = 0;
    while(chr != '\n' && chr != EOF){
        if (isdigit(chr)){
            if (!colFlag)       // Zliczamy tylko jesli zachodzi w pliku przejscie ' '/<znak> --> <cyfra>
                colCount++;
            colFlag = 1;
        }
        else if (chr == ' ')
            colFlag = 0;
        chr = getc(matrix);
    }
    if (chr == EOF){
        fprintf(stderr,"Proba policzenia ilosci kolumn pustego pliku!\n");
        exit(1);
    }
    fseek(matrix, 0, 0);
    return colCount;

}

int count_rows(FILE* matrix){
    int rowLength = get_row_length(matrix);
    int rowCount;
    fseek(matrix, 0, 0);
    char chr = getc(matrix);
    for (rowCount = 0; chr != EOF; rowCount++){     // Skaczemy co linijke i sprawdzamy czy nastepny zczytany znak to bedzie EOF
        fseek(matrix, rowLength, 1);
        chr = getc(matrix);
        fseek(matrix,-1, 1);    // Cofamy sie o zczytany znak
    }
    
    if (rowCount == 0){
        fprintf(stderr,"Proba policzenia ilosci wierszy pustego pliku!\n");
        exit(1);
    }
    fseek(matrix, 0, 0);
    return rowCount;
}

int matrix_elem(FILE* matrix, int row, int col){
    int rowLength = get_row_length(matrix);
    long int offset = row*rowLength + col*size_of_element();
    fseek(matrix, offset, 0);
    int elem;
    fscanf(matrix, "%d", &elem);
    fseek(matrix, 0, 0);
    return elem;
}

void create_empty_output(char* outputName, int rowCount, int colCount){
    FILE* newFile = fopen(outputName,"w");
    char* row = (char*)calloc(colCount*size_of_element() + 2, sizeof(char));
    
    for(int i=0; i < rowCount; i++){
        strcpy(row,"");
        for(int j=0; j < colCount; j++){                // Budujemy pusty wiersz macierzy
            char placeForNumber[size_of_element()];
            strcpy(placeForNumber, "");
            for (int k=0; k < size_of_element(); k++)  // Cale miejsce wypelniamy spacjami
                strcat(placeForNumber," ");
            strcat(row,placeForNumber);
        }
        strcat(row, "\n");
        fwrite(row, sizeof(char), strlen(row), newFile);
    }
    
    free(row);
    
    fclose(newFile);
}

void write_to_matrix(FILE* matrix, int number, int row, int col){
    char numberAsString[size_of_element()];
    sprintf(numberAsString,"%d", number);

    for(int i=strlen(numberAsString); i < size_of_element(); i++)
        strcat(numberAsString," ");

    int rowLength = get_row_length(matrix);
    long int offset = row*rowLength + col*size_of_element();

    fseek(matrix, offset, 0);
    fwrite(numberAsString, sizeof(char), strlen(numberAsString), matrix);
    fseek(matrix, 0, 0);
}

void mul_col(int mulRows, int mulCols, int targetCol, FILE* matA, FILE* matB, FILE* output, int isFormated, int dumpColNo){
    for(int i=0; i < mulRows; i++){
        int matElem = 0;
        for(int j=0; j < mulCols; j++)
            matElem += matrix_elem(matA, i, j) * matrix_elem(matB, j, targetCol);
        
        //printf("%d zapisuje %d do [%d] [%d]", getpid(), matElem, i, dumpColNo);
        isFormated ? write_to_matrix(output, matElem, i, targetCol) : write_to_matrix(output, matElem, i, dumpColNo);
    }
}

