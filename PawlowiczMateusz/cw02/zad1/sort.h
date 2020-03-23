#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#ifndef _sort_h
#define _sort_h

void swap_sys(int fileToSort, int idx1, int idx2, int recordLength, char* recordBuf1, char* recordBuf2);
int partition_sys(int fileToSort, int from, int to, int recordLength);
void qsort_sys(int fileToSort, int from, int to, int recordLength);

void swap_lib(FILE* fileToSort, int idx1, int idx2, int recordLength, char* recordBuf1, char* recordBuf2);
int partition_lib(FILE* fileToSort, int from, int to, int recordLength);
void qsort_lib(FILE* fileToSort, int from, int to, int recordLength);

void sort(const char* filename, int recordCount, int recordLength, int isLib);

#endif