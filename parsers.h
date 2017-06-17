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

/** fibonacci_parser() is responsible for taking the UDP datagram, and converting the
*       string into data that can be processed by the fibonacci function.  It also makes sure
*       not to call the function and return NULL if the string doesn't meet the requirements
*       such as not being 0-300
*/
char * fibonacci_parser(char * buf);

/** roman_numeral_parser() is responsible for taking the UDP datagram, and converting it to
*       a string that can be processed by the roman_numeral_converter function.  It also checks
*       to make sure the returned number is within the range of 0-4000 (max value)
*/
char * roman_numeral_parser(char * buf);

/** hex_convert_parser() is responsible for processing the UDP datagram and calling the
*       appropriate BIG_NUMBER functions to calculate the hex version of the string
*/
char * hex_convert_parser(char * buf);

/** newline_terminator() is a quick little function that gets rid of extra newlines so the
*       server can process data both from the echo, or standalone netcat program
*/
void newline_terminator(char * buf);

#endif
