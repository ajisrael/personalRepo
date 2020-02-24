// ========================================================================= BOF
#include "msp430x22x4.h"   // chip-specific macros & defs
#include "stdint.h"        // MSP430 data type definitions

void main( void )
// -----------------------------------------------------------------------------
// Func:  Initialize & start timer A to generate a PWM waveform & 
//        output it to P2.3/TA1 pin without further CPU intervention.
// Args:  None
// Retn:  None
// -----------------------------------------------------------------------------
{ WDTCTL = WDTPW + WDTHOLD;          // Stop WDT

  P2DIR |= 0x08;                     // P2.3 = Output mode
  P2SEL |= 0x08;                     // P2.3 = alt mode = TA1 = TA OUT1
   
  TACCR0 = 0x2000 - 1;               // Set freq. of PWM (UP mode)
  TACCR1 = TACCR0 + 1 >> 2;          // Set pulse wid. of PWM (UP mode)
   
  TACTL   = TASSEL_2 | ID_0 | MC_1;  // Select SMCLK | div by 1 | UP Mode
  TACCTL1 = CCIS0 | OUTMOD_2;        // Chnl 1 Inp = CCI1B | OUTMOD = Tgl/Rst
                                     // Note:  Compare Mode is default

  _BIS_SR(LPM1_bits | GIE);          // Enter LPM1 w/ IRQs enabled

  // The CPU is now asleep in LPM1 with general IRQs enabled (GIE = 1).
  // The CPU never wakes up because all the work is done by peripheral
  // hardware unit (Timer A, Compare Channel 1, signal OUT1).
}
// ========================================================================= EOF
