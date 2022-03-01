#ifndef EXP_PARSER_H
#define EXP_PARSER_H

#define NO_DECIMAL   // TODO: Change this after adding floats.

#include <stdio.h>
#include <stdlib.h>


double parse_add(void);
int is_digit(void);
int digit();
double number();
double parse_factor();
double parse_mul();
double parse_add();
char* eval(const char* string);
#endif
