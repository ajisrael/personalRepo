// ============================================================================
// Func:  Blink installed Red LED at fixed freq.
// Meth:  Software delay loop excuted in main
// ----------------------------------------------------------------------------

#include "msp430x22x4.h"   // Brobdingnagian chip-specific macros & defs
#include "stdint.h"        // MSP430 data type definitions
#define MAX_TICKS 20000    // Define Blink length (loop passes)

void main(void)  
// ----------------------------------------------------------------------------
// Func:  Init LED on port P1.0, enter wait loop, & toggle P1.0.
// Args:  None
// Retn:  None
// ----------------------------------------------------------------------------
{ WDTCTL = WDTPW | WDTHOLD;                // Stop watchdog timer
  volatile uint16_t i;                     // Generic loop idx.

  P1DIR |=  0x01;                          // Config  P1.0 = output
  P1OUT &= ~0x01;                          // Initial P1.0 = 0 => LED off

  while(1)                                 // Infinite loop
  {
    for (i = 0; i < MAX_TICKS; i++) {} ;   // Empty SWare delay loop
    P1OUT ^= 0x01;                         // Toggle P1.0 => toggle LED
  }
}
// ============================================================================