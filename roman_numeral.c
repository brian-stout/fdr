
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int roman_character_value(char letter);
int roman_numeral_parser(char * rmn_num);

int main (void)
{
    char rmn_num[18];
    printf("%zd\n", sizeof(rmn_num));
    strncpy(rmn_num, "MMMDCCCCLXXXXVIIII", sizeof(rmn_num));
    rmn_num[18] = '\0';
	
    printf("%s\n", rmn_num);
    int number = roman_numeral_parser(rmn_num);
    printf("%d\n", number);
}

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

int roman_numeral_parser(char * rmn_num)
{
    size_t len = strnlen(rmn_num, 18);
    if(len > 18)
    {
        return -1;
    }

    int ret_num = 0;
    int addition = 0;
    int last_val = 1000;

    for(size_t i = 0; i < len; i++)
    {
        addition = roman_character_value(rmn_num[i]);
        if (addition > last_val)
        {
            ret_num = -1;
            break;
        }
        last_val = addition;

        if (addition == 0)
        {
            ret_num = -1;
            break;
        }
        ret_num += addition;
    }

    return ret_num;
}
