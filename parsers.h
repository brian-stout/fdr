#ifndef PARSERS_H
#define PARSERS_H

#define _GNU_SOURCE

#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <openssl/bn.h>

#include "fibonacci.h"
#include "roman_numeral.h"

char * fibonacci_parser(char * buf);
char * roman_numeral_parser(char * buf);
char * hex_convert_parser(char * buf);
void newline_terminator(char * buf);

#endif
