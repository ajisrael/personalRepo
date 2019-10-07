// ========================================================================== BOF
// Orig: 2019.10.3 - Alex Israels
// Rev : 2019.10.4 - Alex Israels
// Func: Reads in data from Nios JTAG UART and writes the ascii value out to the
//       buffer. All the while waiting for when JTAG UART is ready to be read
//       from and written to by checking RVALID bit of the data register and 
//       WSPACE byte of the control register.
// ------------------------------------------------------------------------------

#include <alt_types.h>

void main()
// ------------------------------------------------------------------------------
// Func: Reads in data from Nios JTAG UART and writes the ascii value out to the
//       buffer. All the while waiting for when JTAG UART is ready to be read
//       from and written to by checking RVALID bit of the data register and 
//       WSPACE byte of the control register.
// Args: juAddr   = The base address of the JTAG UART. Has to be manually set.
//       juData   = The value of the data register of the JTAG UART.
//       juCont   = The value of the control register of the JTAG UART.
//       ascii    = 8 bit variable to hold one ascii character.
//       writeBuf = 64 bit buffer to write ascii characters out to.
//       i        = Index counter for writeBuf.
//       done     = Boolian to mark when to stop the process.
// ------------------------------------------------------------------------------
{
   // write out args
   volatile alt_u32 * juAddr = 0x0BB80BB8;     // base address of JTAG UART
            alt_u32   juData = * (juAddr);     // data register of JTAG UART
            alt_u32   juCont = * (juAddr + 1); // control register of JTAG UART
            
            alt_u8    ascii = 0xFF;            // ascii value
            alt_u8    writeBuf[8] = NULL;      // write buffer
            alt_u8    i = 0;                   // position counter
            alt_u8    done = 0;                // boolian for when to stop

   do
   {
      // wait till JTAG UART has a byte to read
      while ((juData & 0x8000) == 0x8000) // 0x8000 masks RVALID bit
      {
         juData = * (juAddr);             // reads data register
      }

      // read an ascii byte from the JTAG UART
      ascii = juData & 0xFF;              // 0xFF copies data bits to ascii

      // if lowercase
      if ((ascii & 0x60) == 0x60)         // verifies ascii == x11x,xxxx
      {
         // make uppercase
         ascii &= ~0x20;                  // ~0x20 clears 5th bit to 0
      }
      // else do nothing

      // wait till JTAG UART has room to recieve byte
      while ((juCont & 0xFFFF0000) == 0x0000FFFF) // check if WSPACE == 0 
      {
         juCont = * (juAddr + 1);         // read control register
      }

      // write the character out to the JTAG UART
      if (ascii == 0x00)                  // check for NULL terminator
      {
         ascii = 0x0A                     // replace with <LF>
         done = 1;                        // mark as done
      }
      writeBuf[i] = ascii;                // write ascii to buffer
      i++;                                // increase buffer index
      if (i > 7) { i = 0; }               // reset i if out of bounds
   }
   while (done == 0)
   // terminate process after writing <LF> character
}
// ========================================================================== EOF