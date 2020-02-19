
#pragma vector=TIMERA1_VECTOR          // Define ISR Vector Table Location.
__interrupt void IsrAdjPwmTA1 (void)   // Define Timer-A PWM Adjust ISR. 
//-----------------------------------------------------------------------------
// Func:  ISR: Adjust Pulse Width for a PWM signal output by TA1.
// Args:  None
// Retn:  None
//-----------------------------------------------------------------------------
{ 
  // ??? Your Local Variable Declarations Go Here

  switch (__even_in_range(TAIV, 10))  // ID IRQ src with optimized switch stmt
  {                 
    case TAIV_TAIFG:                  // Handle IRQ for TAR Rollover to 0
      // ??? Your Code Goes Here.
      break;

    case TAIV_TACCR1:                 // Ignore CC chnl-1 IRQ
    case TAIV_TACCR2:                 // Ignore CC chnl-2 IRQ
    default:                          // Ignore everything else
  }
}
