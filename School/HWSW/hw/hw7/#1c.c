// =============================================================================
// Orig: 2019.10.23 - Alex Israels
// Prob: Problem 1.c
// Func: Calculate the sum of squares from 0 to n.
// Meth: This is accomplished through a recursive function.
// Args: n   = The first n positive integers to calculate the sum of squares.
// Retn: sum = The sum of all the squares.
// -----------------------------------------------------------------------------

#include <alt_types.h>

alt_u32 SumSqr(alt_u32 n)
{
    if (n == 0)    // base case
    {
        return 0;  // return zero
    }
    else           // recursive step
    {
        return n * n + SumSqr(n - 1);  // return sqare plus next recursive call
    }
}
// =============================================================================