// ======================================================================== BOF
// Orig: 2020.02.12 - Alex Israels
// Func: Calculates the frequency of the input signal.
// Meth: Finds the difference between the timings of two successive rising 
//       edges of the input signal. Then uses that value to calculate the
//       frequency of the input signal.
// Defn: DEBNC_DEL  = Debounce delay for the button
//       TACLK_FREQ = The frequency of the TACLK as calculated by its init. 
// Vars: pulsCount  = Keeps track of the number of pulses in the input signal.
//       frequency  = Frequency of the input signal.
//       prevVal    = Previous value of TACCR1.
//       currVal    = Current value of TACCR1.
//       period     = Calculated period of signal in TACLK cycles.
// ----------------------------------------------------------------------------

#include "msp430x22x4.h"
#include "stdint.h"
#include "stdio.h"                     // Allows comm. with host terminal

#define DEBNC_DEL 1500                 // Define Debounce delay (loop passes)
#define TACLK_FREQ 8000000             // Calculated freq of TACLK_FREQ in Hz

volatile uint16_t pulsCount;           // Global Pulse Count. Why volatile?
volatile uint32_t freq;                // Global frequency
volatile uint32_t prevVal;             // Global prevVal of TACCR1
volatile uint32_t currVal;             // Global currVal of TACCR1
volatile uint32_t period;              // Global calculated period of signal

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

        currVal = TACCR1_;              // Get current value of TACCR1

        if (TACTL_ & TAIFG)             // Check for rollover
        {                               // Calculate period with rollover
            period = (0xFFFF + 1 + currVal) - prevVal;
            TACTL_ &= ~TAIFG;           // Reset TAIFG
        }
        else
        {
            period = currVal - prevVal; // Calculate period in clock cycles
        }

        freq = TACLK_FREQ / period;     // Calculate frequency
        prevVal = currVal;              // Set prevVal

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
// Vars:  i = Loop index varialbe for debnounce.
// Retn:  None
// ----------------------------------------------------------------------------
{   volatile uint16_t i;               // Loop idx for debnounce loop

    for(i=0; i < DEBNC_DEL; i++)  {};  // Button press debounce delay

    printf(" %d\n", freq);             // Display pulsCount
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
  freq = 0;                           // Init. frequency of input signal
  prevVal = 0;                        // Init. previous value of TACCR1
  currVal = 0;                        // Init. current value of TACCR1
  period = 0;                         // Init. period of input signal

  _BIS_SR (LPM1_bits | GIE);          // Enter LPM1 w/ IRQs enabled
}                                     // Why LPM1 instead of LPM3?
// ======================================================================== EOF