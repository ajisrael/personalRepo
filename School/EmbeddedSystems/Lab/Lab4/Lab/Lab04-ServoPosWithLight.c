// ============================================================================
// Orig: 2020.02.26 - Collin Palmer
// Func: Moves the position of a servo motor from 0 to 90 degrees linearly 
//       based on the light level return from the ADC10
// Meth: TimerA is configured in UP mode with TACCR0 set to a value to make the
//       period of the PWM signal 20ms. TAIFG is enabled to allow for an ISR to
//       increment or decrement the pulse width (defined by TACCR1) based on
//       the value read from ADC10. 
// Defn: PW_1MS = The value assigned to the TACCR1 register such that a 1ms
//                pulse is generated in UP Mode over a 20ms period.
//       PW_2MS = The value assigned to the TACCR1 register such that a 2ms
//                pulse is generated in UP Mode over a 20ms period.
// Rev:  2020.02.26 - Alex Israels & Collin Palmer
//          - Debugged program to function for lab.
//       2020.03.04 - Alex Israels
//          - Added comments for final submission.
//-----------------------------------------------------------------------------


#include "msp430x22x4.h"
#include "stdint.h"

#define PW_1MS  0x5DC    // 1ms pulse @ 1.2 MHz SMCLK
#define PW_2MS  0xBB8    // 2ms pulse @ 1.2 MHz SMCLK

volatile uint16_t newNadc;   // Hmmm... Why declare a variable volatile 
                             // that is never used in the main program?

static volatile uint16_t stepSize;
static volatile uint8_t  direction = 0xFF;

void main(void) 
//------------------------------------------------------------------------------
// Func:  Init Ports & Interrupts & Configure the ADC10 module of the MSP430,
//        and Timer A for generating a PWM signal to control the servo arm.
// Args:  None
// Retn:  None
//------------------------------------------------------------------------------
{ WDTCTL  = WDTPW + WDTHOLD;  // Stop Watchdog (Yawn);

  DCOCTL  = CALDCO_12MHZ;     // DCO = 12 MHz;
  BCSCTL1 = CALBC1_12MHZ;     // DCO = 12 MHz;
  P3DIR  |=  0x02;            // P3.1 = output (to external LED);
  P3OUT  &= ~0x02;            // Init P3.1 = 0 (LED off)

  ADC10AE0  |= 0x02;          // Enable chnl A0 = P2.0; 
  ADC10CTL0 |= ADC10ON        // Turn ON ADC10
            |  ADC10SHT_2;    // samp-hold tim = 16 cyc;
  ADC10CTL1 |= ADC10SSEL_3    // ADC10CLK source = SMCLK 
            |  ADC10DIV_2     // ADC10CLK divider = ?? (look up, be careful)
            |  INCH_1;        // Select input = chnl A0 (default);
			
    P2DIR |= 0x08;                     // P2.3 = Output mode
    P2SEL |= 0x08;                     // P2.3 = alt mode = TA1 = TA OUT1
    
    TACCR0 = 0x7530 - 1;               // Set freq. of PWM (UP mode) 24KHz
    TACCR1 = TACCR0 - PW_1MS;          // Set pulse wid. of PWM (UP mode)
    
    TACTL   = TASSEL_2 | ID_3          // Select SMCLK | div by 1 
            | MC_1 | TAIE;             // | UP Mode | Enable TAIFG
    TACCTL1 = CCIS0 | OUTMOD_2;        // Chnl 1 Inp = CCI1B | OUTMOD = Tgl/Rst
                                       // Note:  Compare Mode is default
									   
    __bis_SR_register(LPM0_bits | GIE);   // Sleep til return from ADC ISR;

} // end main


#pragma vector = TIMERA1_VECTOR     // Define ISR Vector Table Location.
__interrupt void IsrAdjPwmTA1(void) // Define Timer-A PWM Adjust ISR.
//-----------------------------------------------------------------------------
// Func:  ISR: Adjust Pulse Width for a PWM signal output based on most recent 
//        value from ADC.
// Args:  None
// Retn:  None
//-----------------------------------------------------------------------------
{
    switch (__even_in_range(TAIV, 10)) // ID IRQ src with optimized switch stmt
    {
    case TAIV_TAIFG:                   // Handle IRQ for TAR Rollover to 0
	
	    ADC10CTL0 |= ENC | ADC10SC;    // Enable ADC10 and start sample
		while(ADC10CTL1&ADC10BUSY){}   // Wait for ADC10 to not be busy
		newNadc = ADC10MEM;            // Read new value from ADC10
		TACCR1 = TACCR0 - (newNadc + PW_1MS); // Set PW
        TACTL &= ~TAIFG ;              // Clear TAIFG
        break;

    case TAIV_TACCR1: // Ignore CC chnl-1 IRQ
    case TAIV_TACCR2: // Ignore CC chnl-2 IRQ
    default:          // Ignore everything else
    }
}