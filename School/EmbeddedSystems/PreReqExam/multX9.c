//------------------------------------------------------------------------------
// Orig: 2020.1.22 - Alex Israels
// Prog: multX9.c
// Func: Multiplies a base value by 9.
// Args: base = Base value to be multiplied.
// Retn: result = Result of function.
//------------------------------------------------------------------------------

#include <msp430x22x4.h>

uint32_t MultX9 (uint16_t base)
//------------------------------------------------------------------------------
// Func: Multiplies a base value by 9.
// Args: base = Base value to be multiplied.
// Retn: result = Result of function.
//------------------------------------------------------------------------------
{
    uint32_t result = base + (base << 3);
}