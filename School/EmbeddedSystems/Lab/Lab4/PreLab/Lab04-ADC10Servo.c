 #include "msp430x22x4.h"
#include "stdint.h"

volatile uint16_t newNadc;   // Hmmm... Why declare a variable volatile 
                             // that is never used in the main program?
							 
#define PW_1MS  0x64    // 1ms pulse @ 1 MHz SMCLK
#define PW_2MS  0xC8    // 2ms pulse @ 1 MHz SMCLK

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
        uint_16t check = ADC10CTL1 & ADC10BUSY;
		while(check != ADC10BUSY)
		{
			check = ADC10CTL1 & ADC10BUSY;
		}
		newNadc = ADC10MEM;
        
        TACTL &= ~TAIFG ;              // Clear TAIFG
        break;

    case TAIV_TACCR1: // Ignore CC chnl-1 IRQ
    case TAIV_TACCR2: // Ignore CC chnl-2 IRQ
    default:          // Ignore everything else
    }
}							 
							 
void main(void) 
//------------------------------------------------------------------------------
// Func:  Init Ports & Interrupts &
//        ??? INSERT YOUR DESCRIPTION of main here
// Args:  None
// Retn:  None
//------------------------------------------------------------------------------
{ WDTCTL  = WDTPW + WDTHOLD;  // Stop Watchdog (Yawn);

  DCOCTL  = CALDCO_12MHZ;     // DCO = 12 MHz;
  BCSCTL1 = CALBC1_12MHZ;     // DCO = 12 MHz;
  P3DIR  |=  0x02;            // P3.1 = output (to external LED);
  P3OUT  &= ~0x02;            // Init P3.1 = 0 (LED off)
  
  P2DIR |= 0x08;                     // P2.3 = Output mode
  P2SEL |= 0x08;                     // P2.3 = alt mode = TA1 = TA OUT1

  TACCR0 = 0x07D0 - 1;               // Set freq. of PWM (UP mode) 1 MHz
  TACCR1 = TACCR0-PW_1MS;            // Set pulse wid. of PWM (UP mode)
  stepSize = (PW_2MS-PW_1MS) / 50  ;// Calculate the step size @ PWM freq.

  TACTL   = TASSEL_2 | ID_2          // Select SMCLK | div by 2
		  | MC_1 | TAIE;             // | UP Mode | Enable TAIFG
  TACCTL1 = CCIS0 | OUTMOD_2;        // Chnl 1 Inp = CCI1B | OUTMOD = Tgl/Rst
								     // Note:  Compare Mode is default

  ADC10AE0  |= 0x01;          // Enable chnl A0 = P2.0; 
  ADC10CTL0 |= ADC10ON        // Turn ON ADC10
            |  ADC10IE        // enable ADC IRQ 
            |  ADC10SHT_2;    // samp-hold tim = 16 cyc;
  ADC10CTL1 |= ADC10SSEL_3    // ADC10CLK source = SMCLK 
            |  ADC10DIV_2     // ADC10CLK divider = ?? (look up, be careful)
            |  INCH_0;        // Select input = chnl A0 (default);

  while(1)                                // Do Forever
  { ADC10CTL0 |= ENC | ADC10SC;           // Enab ADC10 & start next sample;
    __bis_SR_register(LPM0_bits | GIE);   // Sleep til return from ADC ISR;
  }

} // end main




