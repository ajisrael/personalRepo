// =============================================================================
// Orig: 2019.10.23 - Alex Israels
// Prob: Problem 1.d
// Func: Main function to call sum of squares function and print to console.
// Meth: Depends on the squares function called.
// Vars: n   = The first n positive integers to calculate the sum of squares.
// Retn: sum = The sum of all the squares.
// -----------------------------------------------------------------------------

#include <alt_types.h>

# define N_MAX 4294967295  // Max value of an unsigned integer

void main()
{
    alt_u32 sum = 0;       // Variable for sum

    for (alt_u32 n = 0; n <= N_MAX; n++) // Iterate over all possible integers
    {
        sum = SumSqr(n);                 // Calculate the sum of squares for n
        printf("Sum: \t%d\n", sum);      // Print to console
    }
    exit(0); 
}
// =============================================================================