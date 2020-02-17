// ======================================================================== BOF
#include "msp430x22x4.h"
#include "stdint.h"
#include "stdio.h"                     // Allows comm. with host terminal

#define DEBNC_DEL 1500                 // Define Debounce delay (loop passes)

volatile uint16_t pulsCount;           // Global Pulse Count. Why volatile?

#pragma vector=TIMERA1_VECTOR
__interrupt void IsrPulsCntTACC1 (void) 
// ----------------------------------------------------------------------------
// Func:  At TACCR1 IRQ, increment Pulse Count & toggle the built-in Red LED. 
// Args:  None
// Retn:  None
// ----------------------------------------------------------------------------
{ 
  switch (__even_in_range(TAIV, 10))    // I.D. source of TA IRQ
  {                 
    case TAIV_TACCR1:                   // handle TA chnl 1 IRQ

        pulsCount++;                    // Increment pulsCount
        P1OUT ^=  0x01;                 // Toggle built-in LED on P1.0
        TAIV  &= ~TAIV_TACCR1;          // Clear IRQ signal before return
        break;
    case TAIV_TACCR2:                   // ignore TA  chnl 2   IRQ for this App
    case TAIV_TAIFG:                    // ignore TAR rollover IRQ for this App
    default:                            // ignore everything else
  }
}


#pragma vector=PORT1_VECTOR             // Define ISR Vector Location 
__interrupt void IrqPort1 (void)        // Define Push Button ISR
// ----------------------------------------------------------------------------
// Func:  IRQ: Print the pulse count when installed push button is pressed
// Args:  None
// Retn:  None
// ----------------------------------------------------------------------------
{
    for(i=0; i < DEBNC_DEL; i++)  {};  // Button press debounce delay

    printf(" %d\n", pulsCount);        // Display pulsCount
    P1IFG &= ~0x04;                    // Clear IRQ signal before return

    for(i=0; i < DEBNC_DEL; i++)  {};  // Button release debounce delay
}

void InitPorts (void)
// ----------------------------------------------------------------------------
// Func:  Initialize Button & LED ports, and ports for I/O on TA1 Capture 
// Args:  None
// Retn:  None
// ----------------------------------------------------------------------------
{
    P1DIR |=  0x01;                   // Set P1.0 as an output

    P2DIR &= ~0x08;                   // Set P2.3 as an input (done by default)
    P2SEL |=  0x08;                   // Select P2.3 to alt fn: TA-CCI1B input

    P1DIR &= ~0x04;                   // Set P1.2 as an input (Button)
    P1REN |=  0x04;                   // Enable P1.2 Pull-Up Resistor 
    P1IE  |=  0x04;                   // Enable P1.2 IRQ 
    P1IES |=  0x04;                   // Configure P1.2 IRQ = falling edge
}

void main(void)
// ----------------------------------------------------------------------------
// Func:  Init I/O ports & IRQs, Config. Timer TA Chnl 1, enter LowPwr Mode
// Args:  None
// Retn:  None
// ----------------------------------------------------------------------------
{ WDTCTL = WDTPW | WDTHOLD;           // Stop Watchdog Timer
   
  InitPorts();                        // Configure I/O Pins
   
  TACTL   = TASSEL_2 | ID_1 | MC_2;   // SMCLK | Div by 2 | Contin Mode
  TACCTL1 = CM0 | CCIS0               // Rising Edge Cap | input = CCI1B 
          | CAP | SCS | CCIE;         // Cap Mode | Sync Cap | Enab IRQ

  pulsCount = 0;                      // Init. input pulse counter

  _BIS_SR (LPM1_bits | GIE);          // Enter LPM1 w/ IRQs enabled
}                                     // Why LPM1 instead of LPM3?
// ======================================================================== EOF