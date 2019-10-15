// -----------------------------------------------------------------------------
// Orig: 2019.10.06 - Alex Israels
// Revs: 2019.10.08 - Alex Israels
// Prog: HelloWorldDIY.c
// Func: Writes Hello World string to terminal via JTAG UART.
// Meth: Write string to data register after checking WSPACE is available.
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

void PutTerm(alt_u8 *strPtr)
// -----------------------------------------------------------------------------
// Func: Print the argument pointer's char string to JTAG UART using pointer
//       arithematic.
// Args: strPtr = pointer to start of char string in memory.
// -----------------------------------------------------------------------------
{ 
   // While current char != NULL
   while (*strPtr != 0x00)
   {
      while ((*ctrlPtr & WSPACE_MASK) == 0) {} // Wait for WSPACE > 0
      *dataPtr = *strPtr;                      // Write char to terminal
      strPtr++;                                // Advance to next char
   }

   // When char = NULL
   while ((*ctrlPtr & WSPACE_MASK) == 0) {}    // Wait for WSPACE > 0
   *dataPtr = 0x0A;                            // Ouptut newline char (^LF)
}

// -----------------------------------------------------------------------------
void main() 
{
   alt_u8  outStr[] = "Hello World :-P " ;   // Declare & init ASCII char string
   PutTerm (outStr) ;                        // Print it to data register
}