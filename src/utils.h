#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#define INPUT_SIZE 256

int read_line(FILE* stream, char* text, size_t size);
int read_text(const char* prompt, char* text, size_t size);
int read_int(const char* prompt, int minimum, int maximum, int* value);

#endif
