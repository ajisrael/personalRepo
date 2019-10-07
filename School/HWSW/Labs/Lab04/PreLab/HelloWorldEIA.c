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
// Func: Print the argument pointer's char string to JTAG UART in EIA.
// Args: strPtr = pointer to start of char string in memory.
// Vars: wspace = upper half word of control register.
// -----------------------------------------------------------------------------
{ asm volatile
   ("Loop:                         \n\t"
    "   ldwio  r8,  0(%[Rstr])     \n\t" // load current char   
    "   beq    r8,  r0, Done       \n\t" // If current char == NULL break
    "   ldhuio r9,  2(%[Rcont])    \n\t" // load wspace
    "   beq    r9,  r0, Loop       \n\t" // If wspace == 0 go to Loop
    "   stwio  %[Rdata], r8        \n\t" // store char to data register
    "   addi   %[Rstr], %[Rstr], 4 \n\t" // increment str ptr
    "   br     Loop                \n\t" // go to loop
    "Done:                         \n\t"
    "   addi   r8, r0, 0x0A        \n\t" // store <LF> to data register
    : [Rdata] "=r" (dataReg)
    : [Rstr] "r" (*strPtr), [Rcont] "r" (contReg)
    : "r8", "r9", "memory"
   );
}

// -----------------------------------------------------------------------------
void main() 
{
   alt_u8  outStr[] = "Hello World :-P " ;   // Declare & init ASCII char string
   PutTerm (outStr) ;                        // Print it to data register
}