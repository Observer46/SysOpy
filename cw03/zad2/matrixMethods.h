#ifndef _matrixMethods_h
#define _matrixMethods_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// UWAGA: Przyjeta konwecja - na kazda liczbe w pliku przeznaczone jest 10 znakow - daje teortycznie liniowy dostep
// Zalozenie - zeby ograniczyc ilosc i/o to w kazdej funkcji plik 'matrix' jest juz otwarty

int isANumber(char* number);        // Akceptuje tylko dodatnie liczby

int size_of_element();
int get_row_length(FILE* matrix);
int count_cols(FILE* matrix);
int count_rows(FILE* matrix);
int matrix_elem(FILE* matrix, int row, int col);
void create_empty_output(char* outputName, int rowCount, int colCount);
void write_to_matrix(FILE* matrix, int number, int row, int col);
void mul_col(int mulRows, int mulCols, int targetCol, FILE* matA, FILE* matB, FILE* output, int isFormated, int dumpColNo);

#endif