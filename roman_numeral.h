#ifndef ROMAN_NUMERAL_H
#define ROMAN_NUMERAL_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int roman_character_value(char letter);
int roman_numeral_converter(char * rmn_num);

#endif
