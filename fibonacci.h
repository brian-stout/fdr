#ifndef FIBONACCI_H
#define FIBONACCI_H
#define _GNU_SOURCE


#include <stdio.h>
#include <stdlib.h>
#include <openssl/bn.h>

/** BN_fibonacci() is a function that calculates the number of the specified iteration
*       of the fibonacci loop.  It employs big_numbers so theoretically you can go
*       way beyond the 300th position, even though the parser hardcaps it there
*/
BIGNUM * BN_fibonacci(int number);

#endif
