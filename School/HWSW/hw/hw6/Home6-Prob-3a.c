// ================================================================PROBLEM 3.(a)
// Orig: 2019.10.18 - Alex Israels
// Prob: Problem 3.(a): Using Pure C code.
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

    alt_u32 irqEnab = 0;            // Reads ctl3 for all enabled interrupts

    // Globally Enable Nios Exceptions and IRQ's
    __builtin_wrctl (0, 0x5);
    
    // Enable both Timer and JTAG-UART IRQ's without altering any other bits.
    irqEnab = __builtin_rdctl (3);
    irqEnab |= 0x81;
    __builtin_wrctl (3, irqEnab);

    // ENable JTAG-UART Write IRQs without altering any other bits.
    *(pJTAG + 1) |= 0x2;

    // DISable JTAG-UART Read IRQs without altering any other bits.
    *(pJTAG + 1) &= 0xFFFFFFFE;

    // Enable Timer TO (rollover) IRQ without altering any other bits.
    *(pTIMR + 2) |= 0x1;

    return;     // Return to global exception handler
}
// =============================================================================