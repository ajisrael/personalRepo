// -----------------------------------------------------------------------------
// Orig: 2019.10.06 - Alex Israels
// Revs: 2019.10.08 - Alex Israels
// Prog: HelloWorldDIY.c
// Func: Writes Hello World string to terminal via JTAG UART.
// Meth: Write string to data register after checking WSPACE is available.
// Vars: outStr  = Hello World string.
//       dataPtr = Pointer to data register.
//       ctrlPtr = Pointer to control register. 
//       dataVal = Value of the data register. Ensures data reg. is read once.
// Defn: BASE_ADDR   0xDEADBEEF = Base address of JTAG UART
//       WSPACE_MASK 0xFFFF0000 = Mask for the WSPACE bits of the cont. reg.
// -----------------------------------------------------------------------------
#  include <alt_types.h>

# define BASE_ADDR   0xDEADBEEF // Base address of JTAG UART
# define WSPACE_MASK 0xFFFF0000 // Mask for the WSPACE bits of the cont. reg.

volatile alt_u32 * dataPtr = (alt_u32*)  BASE_ADDR;      // Ptr to data reg.
volatile alt_u32 * ctrlPtr = (alt_u32*) (BASE_ADDR + 1); // Ptr to cont. reg.

alt_u32 dataVal = 0; // Value of data Register (Only read data reg. once)

void PutTerm(alt_u8 *strPtr)
// -----------------------------------------------------------------------------
// Func: Print the argument pointer's char string to JTAG UART using pointer
//       arithematic.
// Args: strPtr = pointer to start of char string in memory.
// -----------------------------------------------------------------------------
{ 
   while (*strPtr != 0x00)                     // While current char != NULL
   {
      while ((*ctrlPtr & WSPACE_MASK) == 0) {} // Wait for WSPACE > 0
      *dataPtr = (alt_u32) *strPtr;            // Write char to terminal
      strPtr++;                                // Advance to next char
   }                                           // When char = NULL
   *strPtr = 0x0A                              // Ouptut newline char (^LF)
   *dataPtr = (alt_u32) *strPtr;
}

// -----------------------------------------------------------------------------
void main() 
{
   alt_u8  outStr[] = "Hello World :-P " ;   // Declare & init ASCII char string
   PutTerm (outStr) ;                        // Print it to data register
}