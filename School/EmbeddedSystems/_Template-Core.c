// ============================================================================
// Orig: 
// Func: 
// Meth:        
// Rev:  
//-----------------------------------------------------------------------------

#include "msp430x22x4.h"   // Brobdingnagian chip-specific macros & defs
#include "stdint.h"        // MSP430 data type definitions

void main(void)
//-----------------------------------------------------------------------------
// Func: 
// Meth: 
// Args: None
// Retn: None
//-----------------------------------------------------------------------------
{ 
    WDTCTL = WDTPW | WDTHOLD;      // Stop Watchdog Timer

}
// ============================================================================