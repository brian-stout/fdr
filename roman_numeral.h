#ifndef ROMAN_NUMERAL_H
#define ROMAN_NUMERAL_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/** roman_character_value() returns a value based on a letter inputted, or 0
*       if that character is an invalid roman numeral character
*/
int roman_character_value(char letter);

/** roman_numeral_converter() loops through a string of roman numerals and adds them together
*       It disqualifies a string if it's subtractive, repeats 10s characters more than four times
*       or repeats 5s character more than once.  If it breaks any of these rules it returns a
*       negative value
*/
int roman_numeral_converter(char * rmn_num);

#endif
