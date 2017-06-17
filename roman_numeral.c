#include "roman_numeral.h"

int roman_character_value(char letter)
{
    switch(letter)
    {
        case 'M':
            return 1000;
        case 'D':
            return 500;
        case 'C':
            return 100;
        case 'L':
            return 50;
        case 'X':
            return 10;
        case 'V':
            return 5;
        case 'I':
            return 1;
        default:
            return 0;
    }
}

int roman_numeral_converter(char * rmn_num)
{
    //Checks to see if the string doesn't exceed the longest string
    //  possible to be parsed.
    size_t len = strnlen(rmn_num, 18);
    if(len > 18) {
        return -1;
    }
    //If string has nothing in it pretty much, return 0
    if(len == 1) {
        return 0;
    }

    int ret_num = 0;
    int addition = 0;
    int last_val = 1001;

    int four_char_count = 0;

    for(size_t i = 0; i < len; i++)
    {
        //Calculates the character value
        addition = roman_character_value(rmn_num[i]);
        //If the previous character was the same as the current one
        //  make sure it follows all the rules for additive roman_numerals
        if (addition == last_val) {
            //IF it's a five multiples it can't repeat ever
            if (addition == 500 || addition == 50 || addition == 5) {
                ret_num = -1;
                break;
            } else {
                four_char_count++;
                //If it's a tens multiple it can only repeat four times
                if(four_char_count == 4) {
                    ret_num = -1;
                    break;
                }
            }
        //If it's not the same then reset the count
        } else {
            four_char_count = 0;  
        }

        //If the next value is bigger than the last, the characters are out of order
        if (addition > last_val) {
            ret_num = -1;
            break;
        }
        last_val = addition;

        //If it's not a valid roman numeral return a bad number
        if (addition == 0) {
            ret_num = -1;
            break;
        }
        ret_num += addition;
    }

    return ret_num;
}
