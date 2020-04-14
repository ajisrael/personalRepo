//===========================================================================BOF
// FILE FUNC: Configure USCB0 for I2C output to COTS slave DAC chip & 
//            Output exponentially decreasing Sawtooth waveform.
//------------------------------------------------------------------------------

#include "msp430x22x4.h"
#include "stdint.h"
#include "stdio.h"        // For later addition of printf statements.

typedef union             // 16-bit cmd & data word type for DAC
{ uint8_t  u8[2];         // 08-bits for easy write to 8-bit TX-BUF reg
  uint16_t u16;           // 16-bits for easy arithmetic on full word
} DacWord_t;

void main(void)
//------------------------------------------------------------------------------
// Func:  I2C master sends 16-bit words to DAC chip slave.
//        Most Signif   4-bits = DAC Ctrl word. 
//        Least signif 12-bits = data for DAC to convert to analog. 
//        Init Data value = 0x0FFF.
// Args:  None
// Retn:  None
//------------------------------------------------------------------------------
{ WDTCTL = WDTPW + WDTHOLD;            // Stop the watchdog timer

  // Declare & initialize vars for DAC Application Data 
  int16_t i;                           // Decl generic loop counter 
  uint16_t DAC_MAX = 0x0FFF;           // Max 12-bit value 
  DacWord_t dacWordOut;                // Decl DAC word as 16/8 bit union
  dacWordOut.u16 = DAC_MAX;            // Init dacWordOut = Max 12b value
  DacWord_t dacWordIn;                 // Decl var for RX input
  dacWordIn.u16 = 0x0000;              // Init dacWordIn to 0


// WARNING: on the eZ430 card, the cc2500 RF chip is hardwired to USCB0 as
// an SPI slave. We must ensure it is disabled to not interfere w/ I2C ops


  // Config. Port P3 for I2C & disable cc2500 SPI interface
  P3DIR  |= 0x0F;                      // P3.0 & P3.3 = CC2500's SPI CS & SCLK
                                       // P3.1 & P3.2 = output dir for I2C
  P3OUT  |= 0x09;                      // Disable CC2500 => SPI CS & SCLK = 1
  P3SEL  |= 0x06;                      // Config P3.1 & P3.2 as I2C SDA & SCL
  
  
  // Configure USCB0 for I2C Master
  UCB0CTL1 = UCSWRST;                      // Stop (reset) USCB0 state mach
  UCB0CTL0 = UCMODE_3 | UCMST | UCSYNC;    // Mode = I2C, master, synchronous
  UCB0CTL1 = UCSSEL_2;                     // Source = SMCLK
  UCB0BR1  = 0;                            // Set I2C Prescalar (Divisor)
  UCB0BR0  = 12;                           // Set I2C Prescalar (Divisor)
  UCB0I2CSA = 0x0D;                        // Slave address (from datasheet)

  UCB0CTL1 &= ~UCSWRST;                // Enable (unreset) USCI state mach
  while(UCB0STAT & UCBBUSY);           // Check if Bus Busy (carrier sense)
  

  // Send infinite loop of vals to DAC
  while (1)                            // OUTER LOOP: form & Tx dacWordOut
  { UCB0CTL1 |= UCTR + UCTXSTT;		   // Send start bit, Slave-Addr, Write-bit  

    for ( i=1; i>=0; i--)              // INNER LOOP: Tx 2 Bytes, MSB 1st
    { while ( !(IFG2 & UCB0TXIFG)){};  // wait while TxBuf not empty
      UCB0TXBUF = dacWordOut.u8[i];    // send next byte (auto-clr TXIFG) 
    }

    printf("%d   ",dacWordOut.u16);    // Print tx bytes

    while ( !(IFG2 & UCB0TXIFG)){};    // wait while TxBuf not empty
    UCB0CTL1 |= UCTXSTP;    		   // order stop bit after curr. byte
    while(UCB0CTL1 & UCTXSTP);         // wait until stop bit is sent
    
    // ??? Insert your code to read back from DAC the word just sent.
    UCB0CTL1 &= ~UCTR;                // Select Reciever mode
    UCB0CTL1 |= UCTXSTT;              // Send read request

    for ( i=1; i>=0; i--)             // Loop for two bytes from RX
    {
      while ( !(IFG2 & UCB0RXIFG)){}; // Wait for RX buf to fill
      dacWordIn.u8[i] = UCB0RXBUF;    // Save next byte
    }

    while ( !(IFG2 & UCB0RXIFG)){};   // wait while rxBuf is not empty
    UCB0CTL1 |= UCTXSTP;    		      // order stop bit after curr. byte
    while(UCB0CTL1 & UCTXSTP);        // wait until stop bit is sent

    printf("%d\n",dacWordIn.u16);     // print out rx bytes

    // ??? End of your code to read back from DAC the word just sent. 

    dacWordOut.u16 = (dacWordOut.u16  > 0)   // Next output value  =  
                   ?  dacWordOut.u16 >> 1    // Current-value/2  OR
                   :  DAC_MAX;               // Rollover to Max value
  } // end while (1)
} 
//===========================================================================EOF
