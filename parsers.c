#include "parsers.h"

void newline_terminator(char * buf)
{
        //Replaces all newlines with a nullbyte
        int len = strlen(buf);

        for(int i = 0; i < len; i++)
        {
            if (buf[i] == '\n') {
                buf[i] = '\0';
            }
        }
}

char * fibonacci_parser(char * fib_number)
{
    //Prevent strtol from returning a negative number
    if(fib_number[1] == '-') {
        return NULL;
    }
    //Sets up the endptr for strtol for error checking
    char * endptr = NULL;

    //Converts string to a base 10 number
    int number = strtol(fib_number+1, &endptr, 10);

    //Checking endptr for non-digit stuff
    if(endptr) {
        //If the first thing is a 0 it's valid input. STRTOL returns this sometimes
        //  for unknown reasons, probably the stdin new line
        if(endptr[0] == 0) {
            //Resetting to null just to be sure
            endptr = NULL;
            //If it gets here, the data is invalid
        } else {
            return NULL;
        }
    }

    //The range for the value is 0-300 (inclusive)
    if(number > 300) {
        return NULL;
    }

    //Create a big number type
    BIGNUM * big_number;

    //Call BN_fib function to return a big_number of the hex value
    big_number = BN_fibonacci(number);

    //Setting up a pointer to call bn2hex to
    char * hex_string;

    //Converts a big_number into a string representation of a hex value
    hex_string = BN_bn2hex(big_number);

    //Don't need the big_number anymore, so free it
    BN_free(big_number);

    //Find the length of the hex plus room for the addition 0 and x characters
    //  as well as a terminating null byte
    size_t len = strlen(hex_string) + 3;

    //Create another room for the 0x null byte combo
    char *buf = malloc(len);

    //Set the preface to the return string '0x'
    buf[0] = '0';
    buf[1] = 'x';
    //Terminate with a nullbyte because strncat might need a nullbyte to know where to stop?
    //  the man page wasn't clear on this.
    buf[2] = '\0';

    strncat(buf, hex_string, len);

    //Free the OPENSSL created string
    OPENSSL_free(hex_string);

    return buf;
}

char * roman_numeral_parser(char * rom_number)
{
    int number = roman_numeral_converter(rom_number+1);

    //The range is 0 to MMMM (4000)  Anything outside this range should be terminated
    if(number < 0 || number > 4000) {
        return NULL;
    }

    //Create data for string.  Should only need 6 characters, round up to a power of 2
    //  highest hex value supported (4000) is 0xfa0
    char *buf = calloc(1, 8);

    //Using snprintf to format the string
    snprintf(buf, 8, "0x%x", number);

    return buf;
}
char * hex_convert_parser(char * dec_number)
{
    //Creating a big number because 10^20 is well over the limit of ULLONG_MAX
    BIGNUM *big_number;

    //Initializing BN
    big_number = BN_new();

    int length = 0;

    //Converts the string (up to 100000000000000000000) to a big_number
    length = BN_dec2bn(&big_number, dec_number+1);

    //If length is zero dec2bn errored no alpha chars or received no input
    if(length == 0) {
        BN_free(big_number);
        return NULL;
    }

    //If the length is 1 but the first digit is 0 than the user inputed D0
    //  this entire portion is just to get 0x0 as an output.
    //By default it outputs 0x if you let the rest of the function run
    if(length == 1 && dec_number[1] == '0') {
    char *buf = malloc(4);
        buf[0] = '0';
        buf[1] = 'x';
        buf[2] = '0';
        buf[3] = '\0';
        BN_free(big_number);
        return buf;
    }
    //The max number 10^20 is 21 digits long, if it exceeds this it exceeds the range
    if(length > 21) {
        return NULL;
    //If the number is 21 digits long we have to make sure the 21st digit is one
    //  and every other digit is 0
    } else if (length == 21) {
        if(dec_number[1] - '0' > 1) {
            BN_free(big_number);
            return NULL;
        }
        for(int i = 2; i <= 21; i++)
        {
            if((dec_number[i] - '0') > 0) {
                BN_free(big_number);
                return NULL;
            }
        }
    }

    //If all that's good convert it to a hex string
    char * hex_string;
    hex_string = BN_bn2hex(big_number);
    BN_free(big_number);

    size_t len = strlen(hex_string) + 4;

    char *buf = malloc(len);

    snprintf(buf, 8, "0x%s", hex_string);

    OPENSSL_free(hex_string);

    return buf;
}

