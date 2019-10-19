// =============================================================== PROBLEM 3.(c)
// Orig: 2019.10.18 - Alex Israels
// Prob: Problem 3.(c): IRQ Source Parser for interrupts in Pure C.
// Func: USING the default DE2 IRQ bit assignments (Lec II-2, Slide 13)
//
//       IF the IRQ was caused by the Timer, then call ISR function IsrTimr,
//       IF the IRQ was caused by the JTAG-UART, then call function IsrJtag,
//
//       IF both IRQs have been asserted, give top priority to the Timer,
//          then service the JTAG-UART after return from servicing the Timer.
// -----------------------------------------------------------------------------
#include <alt_types.h>

void IrqParser(void) // called by global exception handler
{
    alt_u32 irqPend = 0;          // Contents of ctrl reg 4

    irqPend = __builtin_rdctl(4); // Read pending irqs

    if (irqPend & 0x81)           // If both are pending
    {
        IsrTimr();		  // Run timer irq first
        IsrJtag();		  // Then JTAG irq
    }
    else if (irqPend & 0x1)	  // Else if timer pending
    {
        IsrTimr();		  // Run timer irq
    }
    else if (irqPend & 0x80)      // Else if JTAG pending
    {
        IsrJtag();		  // Run JTAG
    }
    return;     // Return to global exception handler
}
// =============================================================================