// -----------------------------------------------------------------------------
// Orig: 2019.10.06 - Alex Israels
// Revs: 2019.10.08 - Alex Israels
// Prog: HelloWorldEIA.c
// Func: Writes Hello World string to terminal via JTAG UART using EIA.
// Meth: Write string to data reg. after checking if WSPACE is available in EIA.
// Vars: outStr  = Hello World string.
//       dataPtr = Pointer to data register.
//       ctrlPtr = Pointer to control register.
// Defn: BASE_ADDR   0xDEADBEEF = Base address of JTAG UART
//       WSPACE_MASK 0xFFFF0000 = Mask for the WSPACE bits of the cont. reg.
// -----------------------------------------------------------------------------
#  include <alt_types.h>

# define BASE_ADDR   0xDEADBEEF // Base address of JTAG UART
# define WSPACE_MASK 0xFFFF0000 // Mask for the WSPACE bits of the cont. reg.

volatile alt_u32 * dataPtr = (alt_u32*)  BASE_ADDR;      // Ptr to data reg.
volatile alt_u32 * ctrlPtr = (alt_u32*) (BASE_ADDR + 1); // Ptr to cont. reg.

void PutTerm(alt_u8 * strPtr)
// -----------------------------------------------------------------------------
// Func: Print the argument pointer's char string to JTAG UART in EIA.
// Args: strPtr = pointer to start of char string in memory.
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
    : [Rstr] "r" (strPtr), [Rcont] "r" (contReg)
    : "r8", "r9", "memory"
   );
}

// -----------------------------------------------------------------------------
void main() 
{
   alt_u8  outStr[] = "Hello World :-P " ;   // Declare & init ASCII char string
   PutTerm (outStr) ;                        // Print it to data register
}