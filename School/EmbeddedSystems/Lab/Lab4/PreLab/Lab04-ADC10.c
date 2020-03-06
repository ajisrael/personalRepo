#include "msp430x22x4.h"
#include "stdint.h"

volatile uint16_t newNadc;   // Hmmm... Why declare a variable volatile 
                             // that is never used in the main program?
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


#pragma vector=ADC10_VECTOR
__interrupt void IsrAdc10LedSwitch(void)
//------------------------------------------------------------------------------
// Func:  ADC10 ISR. ??? INSERT YOUR DESCRIPTION of ISR function here
// Args:  None
// Retn:  None
//------------------------------------------------------------------------------
{
  //  ??? INSERT YOUR CODE HERE to process ADC input & Generate Output
  uint16_t turnOnNadc  = 0x00FF; // Thresholds for LED output need to calibrate
  uint16_t turnOffNacd = 0x0FFF;
  
  newNadc = ADC10MEM;
  
  if (newNadc < turnOnNadc)
  {
	P3OUT |= 0x02; // Turn on LED
  }
  
  if (newNadc > turnOffNacd)
  {
	P3OUT &= ~0x02; // Turn off LED
  }
  
  
  __bic_SR_register_on_exit(LPM0_bits);  // Clr previous Low Pwr bits on stack  
                                         // to keep CPU awake upon return;
} // end ISR

