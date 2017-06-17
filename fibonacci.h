#ifndef FIBONACCI_H
#define FIBONACCI_H
#define _GNU_SOURCE


#include <stdio.h>
#include <stdlib.h>
#include <openssl/bn.h>

BIGNUM * BN_fibonacci(int number);

#endif
