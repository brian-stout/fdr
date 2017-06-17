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
    size_t len = strnlen(rmn_num, 18);
    if(len > 18) {
        return -1;
    }
    if(len == 1) {
        return 0;
    }

    int ret_num = 0;
    int addition = 0;
    int last_val = 1001;

    int four_char_count = 0;

    for(size_t i = 0; i < len; i++)
    {
        addition = roman_character_value(rmn_num[i]);
        if (addition == last_val) {
            if (addition == 500 || addition == 50 || addition == 5) {
                ret_num = -1;
                break;
            } else {
                four_char_count++;
                if(four_char_count == 4) {
                    ret_num = -1;
                    break;
                }
            }    
        } else {
            four_char_count = 0;  
        }

        if (addition > last_val) {
            ret_num = -1;
            break;
        }
        last_val = addition;

        if (addition == 0) {
            ret_num = -1;
            break;
        }
        ret_num += addition;
    }

    return ret_num;
}
