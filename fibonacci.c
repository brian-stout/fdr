#include "fibonacci.h"

BIGNUM * BN_fibonacci(int number)
{

    //Initialize BIGNUM structures
    BIGNUM * prev = BN_new();
    BIGNUM * current = BN_new();
    BIGNUM * next = BN_new();

    //Prev and current need to start as zero and one respectiively
    BN_zero(prev);
    BN_one(current);

    //TODO: Unwrap first two iterations of the loop
    for(int i = 0; i < number + 1; i++)
    {
        if(i == 0) {
            BN_zero(next);
        } else if (i == 1) {
            BN_one(next);
        //Main fibonacci sequence
        } else {
            //next = prev + current
            BN_add(next, prev, current);
            //prev = current
            prev = BN_copy(prev, current);
            //current = next
            current = BN_copy(current, next);
        }
    }
    BN_free(current);
    BN_free(prev);
    return next;
}

