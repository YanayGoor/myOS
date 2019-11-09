#ifndef UTIL_H
#define UTIL_H

#include "../cpu/types.h"

void memory_copy(char *source, char *dest, int nbytes);
void memory_set(char *source, char byte, int nbytes);
void int_to_ascii(int n, char str[]);
void uint_to_ascii(unsigned int n, char str[]);
int strlen(char str[]);
void print_int(int a);
void print_uint(unsigned int a);
#endif