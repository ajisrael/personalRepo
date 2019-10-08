// -----------------------------------------------------------------------------
// Orig: 2019.10.06 - Alex Israels
// Revs: 2019.10.08 - Alex Israels
// Prog: EchoTerm.c
// Func: Echoes input from terminal to terminal via JTAG UART.
// Meth: Read string from data regiser to buffer once RVALID bit is set,
//       and write string to data register after checking WSPACE is available.
// Vars: bufPtr  = Arbitrarily large buffer for holding string.
//       dataPtr = Pointer to data register.
//       ctrlPtr = Pointer to control register. 
//       dataVal = Value of the data register. Ensures data reg. is read once.
// Defn: BASE_ADDR   0xDEADBEEF = Base address of JTAG UART
//       RVALID_MASK 0x00008000 = Mask for the RVALID bit of the data register
//       WSPACE_MASK 0xFFFF0000 = Mask for the WSPACE bits of the cont. reg.
//       DATA_MASK   0x000000FF = Mask for the data bits of the data register
// -----------------------------------------------------------------------------
#  include <alt_types.h>

# define BASE_ADDR   0xDEADBEEF // Base address of JTAG UART
# define RVALID_MASK 0x00008000 // Mask for the RVALID bit of the data register
# define WSPACE_MASK 0xFFFF0000 // Mask for the WSPACE bits of the cont. reg.
# define DATA_MASK   0x000000FF // Mask for the data bits of the data register

volatile alt_u32 * dataPtr = (alt_u32*)  BASE_ADDR;      // Ptr to data reg.
volatile alt_u32 * ctrlPtr = (alt_u32*) (BASE_ADDR + 1); // Ptr to cont. reg.

alt_u32 dataVal = 0; // Value of data Register (Only read data reg. once)

alt_u8* GetTerm(alt_u8 *strPtr)
// -----------------------------------------------------------------------------
// Func: Print the argument pointer's char string to JTAG UART using pointer
//       arithematic.
// Args: strPtr   = Pointer to char string in memory.
// Vars: startPtr = Pointer to start of char string in memory.
// -----------------------------------------------------------------------------
{ 
   alt_u8 reading = 1;          // Boolean for reading loop

   while (reading)                         // While current char != <LF>
   {
      while ((dataVal & RVALID_MASK) == 0) // Wait for RVALID 
      {                                    // If RVALID = 0
         dataVal = *dataPtr;               // Read data register
      }                                    // Else
      dataVal &= DATA_MASK;                // Get char from terminal
      if (dataVal == 0x0A)                 // If it's a <LF>
      {
         *strPtr = 0x00;                   // Ouptut null char (NULL)
         reading = 0;                      // And stop the loop
      }
      else                                 // Continue reading and
      {
         *strPtr = (alt_u8) dataVal;       // Save data to string
         strPtr++;                         // Advance to next char of string
      }
   }
   return startPtr;                        // Return start of string for write
}
// -----------------------------------------------------------------------------

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
   alt_u8 bufPtr[256];     // Arbitrarily large to avoid overflow
   while (1)               // Forever:
   {
      GetTerm (bufPtr);    // Read string from terminal
      PutTerm (bufPtr);    // Echo string back to terminal
   }
}