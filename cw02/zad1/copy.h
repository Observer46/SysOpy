#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#ifndef _copy_h
#define _copy_h

void copy_sys(int sourcefile, int directionfile, int recordCount, int bufferSize);
void copy_lib(FILE* sourcefile, FILE* directionfile, int recordCount, int bufferSize);

void copy(const char* sourcefile, const char* directionfile, int recordCount, int bufferSize, int isLib);

#endif