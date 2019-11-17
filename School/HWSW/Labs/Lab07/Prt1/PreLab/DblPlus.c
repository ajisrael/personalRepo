#include <alt_types.h>

alt_u32 DblPlus(alt_u32 n, alt_u32 * spMin)
{
    alt_u32 * spMin  = NULL;   // Minimum stack ptr value

    // Save SP value
    asm volatile
    (
        "stw r27, 0(%[Rmin]) \n\t"
        :[Rmin] "=r" (spMin)
        :
        :"memory"
    );

    return 0;
}

alt_u32 DblPlus(alt_u32 n, alt_u32 * spMin)
{
    alt_u32 sum = 0;
    // Save SP value
    asm volatile
    (
        "stw r27, 0(%[Rmin]) \n\t"
        :[Rmin] "=r" (spMin)
        :
        :"memory"
    );

    for (alt_u32 i = 0; i < n; i++)
    {
        sum += i + 2 * (i - 1);
    }

    return sum;
}

alt_u32 DblPlus(alt_u32 n, alt_u32 * spMin)
{
    alt_u32 sum = 0;
    
    // Save SP value
    asm volatile
    (
        "stw r27, 0(%[Rmin]) \n\t"
        :[Rmin] "=r" (spMin)
        :
        :"memory"
    );

    if (n > 0)
    {
        sum = DblPlus(n-1, spMin);
    }
    else
    {
        sum = 0;
    }

    return sum;
}