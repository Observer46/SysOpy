#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef _generate_h
#define _generate_h



char* create_record(int length);
void generate(const char* filename, int recordCount, int recordLength);

#endif