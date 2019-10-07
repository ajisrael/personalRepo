// -----------------------------------------------------------------------------
// Orig: 2019.10.06 - Alex Israels
// Prog: HelloWorldDIY.c
// Func: Output a Hello World char. string to terminal via JTAG UART.
// Meth: Write char to data register after checking wspace is available.
// Vars: baseAddr = base address of JTAG UART.
//       dataReg = data register of JTAG UART.
//       contReg = control register of JTAG UART.
// -----------------------------------------------------------------------------
#  include <alt_types.h>

volatile alt_u32 * baseAddr =  0xdeadbeef; // base address of JTAG UART

alt_u32   dataReg  =  * baseAddr;          // data register of JTAG UART
alt_u32   contReg  = (* baseAddr + 1)''    // control register of JTAG UART

void PutTerm(alt_u8 *strPtr)
// -----------------------------------------------------------------------------
// Func: Print the argument pointer's char string to JTAG UART using pointer
//       arithematic.
// Args: strPtr = pointer to start of char string in memory.
// Vars: wspace = upper half word of control register.
// -----------------------------------------------------------------------------
{ 
   while (*strPtr != 0x00)     // While current char != NULL
   {
      alt_u16 * wspace = &contReg[16]; // get current wspace val
      if (wspace > 0)
      {
         dataReg = *strPtr     // Write one char to data reg
         strPtr++;             // Advance to next char
      }
   }
   dataReg = 0x0A              // Ouptut newline char (^LF)
}

// -----------------------------------------------------------------------------
void main() 
{
   alt_u8  outStr[] = "Hello World :-P " ;   // Declare & init ASCII char string
   PutTerm (outStr) ;                        // Print it to data register
}