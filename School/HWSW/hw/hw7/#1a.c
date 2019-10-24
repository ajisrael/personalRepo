// =============================================================================
// Orig: 2019.10.23 - Alex Israels
// Prob: Problem 1.a
// Func: Calculate the sum of squares from 0 to n.
// Meth: This is accomplished through an iterative function.
// Args: n   = The first n positive integers to calculate the sum of squares.
// Retn: sum = The sum of all the squares.
// -----------------------------------------------------------------------------

#include <alt_types.h>

alt_u32 SumSqr(alt_u32 n)
{
    alt_u32 sum = 0; // sum of all squares
    for (alt_u32 i = 0; i <= n; i++) // Iterate through all numbers <= n
    {
        sum += i * i;               // Sum up their squares
    }
    return sum;      // Return
}
// =============================================================================