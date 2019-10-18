/// ================================================================PROBLEM 3.(b)
// Orig: 2019.10.18 - Alex Israels
// Prob: Problem 3.(b): Using EIA embedded in a C function.
// Func: USING the default DE2 IRQ bit assignments (Lec II-2, Slide 13)
//       (1) In the Nios ctrl regs:
//         - Globally Enable Nios Exceptions and IRQs,
//         - Enable both Timer & JTAG-UART IRQs without altering any other bits.
//       (2) ENable JTAG-UART Write IRQs without altering any other bits.
//       (3) DISable JTAG-UART Read IRQs without altering any other bits.
//       (4) ENable Timer TO (rollover) IRQ without altering any other bits.
// -----------------------------------------------------------------------------
#include <alt_types.h>

void EnableIRQs()
{
    volatile alt_u16 *pTIMR = (alt_u16 *)0xBEEF; // base addr. of TIMER
    volatile alt_u32 *pJTAG = (alt_u32 *)0xDEAD; // base addr. of JTAG UART

    asm volatile // You may not need ANY C variables
        (
            "ori   ctl0, ctl0, 0x5      \n\t" // Globally Enable Exceps and IRQs
            "ori   ctl3, ctl3, 0x81     \n\t" // Enable both Timer & JTAG-UART 
            "ldwio r8, 4(%[Rju])        \n\t" // Load JTAG ctrl reg
            "ori   r8, r8, 0x2          \n\t" // Enable JTAG Write IRQ
            "andi  r8, r8, 0xFFFFFFFE   \n\t" // Disable JTAG Read IRQ
            "stwio r8, 4(%[Rju)         \n\t" // Write to JTAG ctrl reg
            "ldw   r8, 8(%[Rtim])       \n\t" // Load timer ctrl reg
            "ori   r8, r8, 0x1          \n\t" // Enable timer TO IRQ
            "stw   r8, 8(%[Rtim])       \n\t" // Write to timer ctrl reg
            :
            :[Rju] "r" (pJTAG), [Rtim] "r" (pTIMR)
            :"r8", "memory"
        );
}
// =============================================================================
