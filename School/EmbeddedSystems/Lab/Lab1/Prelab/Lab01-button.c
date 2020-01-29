// ============================================================================
// Func: Toggle external LED on P2.1 when installed button is pushed
// Meth: Respond to button IRQ on pin P1.2.
//-----------------------------------------------------------------------------

#include "msp430x22x4.h"   // Brobdingnagian chip-specific macros & defs
#include "stdint.h"        // MSP430 data type definitions
#define DEBNC_DEL 1500     // Define Debounce delay (loop passes)

#pragma vector=PORT1_VECTOR          // Declare ISR Vector Location 
__interrupt void IrqButton1 (void)   // Declare Push Button ISR
//-----------------------------------------------------------------------------
// Func: Toggle output on Pin 2.1, when triggered.
// Trig: Pin P1.2 Falling Edge
//-----------------------------------------------------------------------------
{ volatile uint16_t i;                 // Loop idx for debnounce loop

  for(i=0; i < DEBNC_DEL; i++)  {};    // Button press debounce delay

  P2OUT ^= 0x02;                       // Toggle P2.1 => toggle LED
  while (! (P1IN & 0x04))       {};    // Wait for button release

  for(i=0; i < DEBNC_DEL; i++)  {};    // Button release debounce delay

  P1IFG &= ~0x04;                      // Clear P1 IRQ Flag
}

void main (void)
//-----------------------------------------------------------------------------
// Func: Init Button & LED ports, then enter Low Pwr Mode awaiting P1 IRQ
// Args: None
// Retn: None
//-----------------------------------------------------------------------------
{ WDTCTL = WDTPW | WDTHOLD;      // Stop Watchdog Timer

  P2DIR |=  0x02;                // Config  P2.1 = Output (LED)
  P2OUT &= ~0x02;                // Initial P2.1 = 0 => LED off

  P1DIR &= ~0x04;                // Config P1.2 = Input (Button)
  P1REN |=  0x04;                // Enable P1.2 Pull-Up Resistor 
  P1IE  |=  0x04;                // Enable P1.2 IRQ 
  P1IES |=  0x04;                // Config P1.2 IRQ = falling edge  
  
  _BIS_SR (LPM3_bits | GIE);     // Enter Low Pwr Mode 3 w/ IRQs enab

  // The CPU is now asleep in LPM3 with general IRQs enabled (GIE = 1) 
  // It wakes up when it gets an IRQ, executes the ISR, then
  // goes back to sleep upon return from the ISR. 
}
// ============================================================================