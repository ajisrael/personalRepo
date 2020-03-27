//========================================================================== BOF
// FILE FUNC: Configure USCB0 for SPI output to COTS slave DAC chip & 
//            Continuously output an exponentially decreasing Sawtooth wave 
//------------------------------------------------------------------------------

#include "msp430x22x4.h"   // Brobdingnagian chip-specific macros & defs
#include "stdint.h"        // MSP430 integer data type definitions

typedef union              // Control & Data word type for DAC
{ uint8_t  u8[2];          // 08-bits for easy write to 8-bit TX-BUF reg.
  uint16_t u16;            // 16-bits for easy arithmetic on full word
} dacWord_t;

 void main(void)
//------------------------------------------------------------------------------
// Func:  SPI 3-wire master sends 16-bit words DAC chip slave.
//        Most  Signif  4-bits = DAC Control word = 0. 
//        Least signif 12-bits = data val to be converted to analog voltage. 
//        Init Data value = 0x0FFF = Max possible 12-bit value.
//        Subsequent vals = (previous value)/2  with rollover at 0. 
// Args:  None
// Retn:  None
//------------------------------------------------------------------------------
{ WDTCTL = WDTPW + WDTHOLD;    // Stop the watchdog timer

  // Config. GPIO Ports for SPI
  P3SEL  =  0x0A;               // P3.1 & P3.3 = alt modes = SPI SIMO & SCLK 
  P3DIR  =  0x0A;               // P3.1 & P3.3 = output dir

  P3SEL &= ~0x01;               // P3.0  = GPIO for SPI SS
  P3DIR |=  0x01;               // P3.0  = output dir
  P3OUT |=  0x01;               // init. = clear SPI SS

  // Config. USCB0 for SPI Master
  BCSCTL3  |= LFXT1S_2;                 // ACLK Source = VLO (12 KHz)
  UCB0CTL0 |=  UCSYNC | UCMST | UCMSB;  // 3-pin SPI master, Send msb 1st
  UCB0CTL1 |=  UCSSEL_1;                // SCLK Source Select
  UCB0BR1   =  0x00;                    // SCLK Divider MSB
  UCB0BR0   =  0x50;                    // SCLK Divider LSB

  // Declare vars for DAC Application 
  int8_t   i;                           // Decl. generic loop counter 
  uint16_t DAC_MAX = 4095;              // Defn. max possible 12 bit value 
  dacWord_t dacWordOut;                 // Decl. 16/8-bit union for DAC Command
  dacWordOut.u16 = DAC_MAX;             // Init. DAC Command = Max value 

  UCB0CTL1 &= ~UCSWRST;                 // Enable (Un-reset) USCB0
  while (UCB0STAT & UCBUSY){};          // Wait til SPI is idle (should be now)

  // Send continuous series of values to DAC
  while (1)                                 // OUTER LOOP: set & Tx dacWordOut 
  { P3OUT &= ~0x01;                         // Assert SS (active low)

    for ( i=1; i>=0; i--)                   // INNER LOOP: Tx 2 Bytes, MSB 1st
    { UCB0TXBUF = dacWordOut.u8[i];         // Tx next byte (TXIFG auto-clr) 
      while ( !(IFG2 & UCB0TXIFG)){};       // Wait til TXBUF is ready (empty) 
    }
    while (UCB0STAT & UCBUSY){};            // Wait til ALL bits shift out SPI
    P3OUT |= 0x01;                          // De-assert SS => End of stream

    dacWordOut.u16 = (dacWordOut.u16  > 0)  // If prev value > 0
                   ?  dacWordOut.u16 >> 1   // Then output (prev value)/2
                   :  DAC_MAX;              // else rollover to Max value
  } // end while(1)
} 
//========================================================================== EOF
  


                                   