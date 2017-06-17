#include "fibonacci.h"

BIGNUM * BN_fibonacci(int number)
{
    BIGNUM * prev = BN_new();
    BIGNUM * current = BN_new();
    BIGNUM * next = BN_new();

    BN_zero(prev);
    BN_one(current);

    for(int i = 0; i < number + 1; i++)
    {
        if(i == 0) {
            BN_zero(next);            
        } else if (i == 1) {
            BN_one(next);
        } else {

            BN_add(next, prev, current);
            prev = BN_copy(prev, current);
            current = BN_copy(current, next);
        }
    }

    BN_free(current);
    BN_free(prev);
    return next;
}

