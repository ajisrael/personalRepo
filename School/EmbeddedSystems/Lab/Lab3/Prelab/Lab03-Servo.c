// ============================================================================
// Orig: 2020.02.19 - Alex Israels
// Func: Moves the position of a servo motor from 0 to 90 degrees linearly over
//       1 second and then moves it back to 0 in one second
// Meth: TimerA is configured in UP mode with TACCR0 set to a value to make the
//       period of the PWM signal 20ms. TAIFG is enabled to allow for an ISR to
//       increment or decrement the pulse (defined by TACCR1) by the calculated 
//       step when the program is initalized. When the value of TACCR1 is
//       outside the 1-2ms range the direction is flipped and the pulse width
//       changes to move the arm in the desired direction.
// Defn: PW_1MS = The value assigned to the TACCR1 register such that a 1ms
//                pulse is generated in UP Mode over a 20ms period.
//       PW_2MS = The value assigned to the TACCR1 register such that a 2ms
//                pulse is generated in UP Mode over a 20ms period.
// Rev:  None (yet)
//-----------------------------------------------------------------------------

#include "msp430x22x4.h"   // chip-specific macros & defs
#include "stdint.h"        // MSP430 data type definitions

#define PW_1MS = 0x5910    // 1ms pulse @ 1.2 MHz SMCLK
#define PW_2MS = 0x5460    // 2ms pulse @ 1.2 MHz SMCLK

static volatile uint16_t stepSize;
static volatile uint8_t  direction = 0xFF;

#pragma vector = TIMERA1_VECTOR     // Define ISR Vector Table Location.
__interrupt void IsrAdjPwmTA1(void) // Define Timer-A PWM Adjust ISR.
//-----------------------------------------------------------------------------
// Func:  ISR: Adjust Pulse Width for a PWM signal output by TA1.
// Args:  None
// Retn:  None
//-----------------------------------------------------------------------------
{
    // ??? Your Local Variable Declarations Go Here

    switch (__even_in_range(TAIV, 10)) // ID IRQ src with optimized switch stmt
    {
    case TAIV_TAIFG:                   // Handle IRQ for TAR Rollover to 0
        if (direction)                 // Increase when direction != 0
        {
            TACCR1 += stepSize
        }
        else                           // Decrease when direction == 0
        {
            TACCR1 -= stepSize
        }

        if (TACCR1 >= PW_1MS || TACCR1 <= PW_2MS)   // Check boundries
        {
            direction ^= 0xFF;         // Update direction
        }
        
        TACTL &= ~TAIFG                // Clear TAIFG
        break;

    case TAIV_TACCR1: // Ignore CC chnl-1 IRQ
    case TAIV_TACCR2: // Ignore CC chnl-2 IRQ
    default:          // Ignore everything else
    }
}

void main(void)
//-----------------------------------------------------------------------------
// Func: Initialize & start timer A to generate a PWM waveform & 
//       output it to P2.3/TA1 pin without further CPU intervention.
// Meth: 
// Args: None
// Retn: None
//-----------------------------------------------------------------------------
{ 
    WDTCTL = WDTPW | WDTHOLD;          // Stop Watchdog Timer

    P2DIR |= 0x08;                     // P2.3 = Output mode
    P2SEL |= 0x08;                     // P2.3 = alt mode = TA1 = TA OUT1
    
    TACCR0 = 0x5DC0 - 1;               // Set freq. of PWM (UP mode) 24KHz
    TACCR1 = PW_1MS;                   // Set pulse wid. of PWM (UP mode)
    stepSize = (TACCR0 - TACCR1) / 50  // Calculate the step size @ PWM freq.
    
    TACTL   = TASSEL_2 | ID_0          // Select SMCLK | div by 1 
            | MC_1 | TAIE;             // | UP Mode | Enable TAIFG
    TACCTL1 = CCIS0 | OUTMOD_2;        // Chnl 1 Inp = CCI1B | OUTMOD = Tgl/Rst
                                       // Note:  Compare Mode is default

    _BIS_SR(LPM1_bits | GIE);          // Enter LPM1 w/ IRQs enabled

    // The CPU is now asleep in LPM1 with general IRQs enabled (GIE = 1).
    // The CPU never wakes up because all the work is done by peripheral
    // hardware unit (Timer A, Compare Channel 1, signal OUT1).

}
// ============================================================================
