// =========================================================================BOF
// Orig: 2020.02.24 - Collin Palmer
// Func: Turn on and off an LED based on the ambient light in the room. Night
//       Light.
// Meth: Using the ADC to measure voltage from a voltage divider circuit using 
//       a photo resistor to convert the relative ammount of light in the room
//       to a measurable voltage. The voltage is then compared to precalculated
//       thresholds to turn on or off the LED.
// Vars: newNadc = The most recent value read from the ADC
// Rev:  2020.02.26 - Alex Israels & Collin Palmer
//          - Debugged program to function for lab.
//       2020.03.04 - Alex Israels
//          - Added comments for final submission.
//-----------------------------------------------------------------------------
#include "msp430x22x4.h"
#include "stdint.h"

volatile uint16_t newNadc;   // Hmmm... Why declare a variable volatile 
                             // that is never used in the main program?
void main(void) 
//------------------------------------------------------------------------------
// Func:  Init Ports & Interrupts & the ADC, then enter an infinite loop to
//        keep sampling the ADC indefinitely.
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
// Func:  ADC10 ISR. Compare the value of Nadc with predefined thresholds
//        to turn off or on the LED based on those thresholds.
// Trig:  Triggered by the ADC10 IFG
// Meth:  If the value of Nadc is greater than the turn on threshold, then turn
//        on the LED. If it is less than the turn off threshold, then turn off
//        the LED.
// Args:  None
// Vars:  turnOnNadc  = The threshold by which the LED should turn on if Nadc 
//                      exceeds this value.
//        turnOffNacd = The threshold by which the LED should turn off if Nadc
//                      exceeds this value.
// Retn:  None
//------------------------------------------------------------------------------
{
  uint16_t turnOnNadc  = 0x1C7; // Thresholds for LED output
  uint16_t turnOffNacd = 0x39;
  
  newNadc = ADC10MEM;           // Read the new value from the ADC
  
  if (newNadc > turnOnNadc)     // Check new value against turn on threshold
  {
	P3OUT |= 0x02;          // Turn on LED
  }
  
  if (newNadc < turnOffNacd)    // Chekc new value against turn off threshold
  {
	P3OUT &= ~0x02;         // Turn off LED
  }
  
  __bic_SR_register_on_exit(LPM0_bits);  // Clr previous Low Pwr bits on stack  
                                         // to keep CPU awake upon return;
} // end ISR
// =========================================================================EOF