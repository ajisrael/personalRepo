//------------------------------------------------------------------------------
// Orig: 2019.10.15 - Alex Israels
// Prog: ButtonTimPoll.c
// Func: Calculates the number of clock cycles between button presses.
// Asum: A Nios system with an input pIO for the four DE2 pushbuttons, and a
//       fixed period timer (i.e., Qsys defined the period, with the "Writable
//       Period" option set to OFF).
// Meth: In an infinite loop:
//       1. Initialize the timer to its preset max value by writing (anything) 
//          to a period register. 
//       2. Wait for a falling edge on pushbutton KEY[3]
//          a. Poll the edge capture reg. of the pushbutton PIO
//          b. NEED to set edge capture reg. to "falling edge" in QSys.
//          c. Clear register after detecting an edge.
//       3. Start the timer.
//       4. Wait for another falling edge on KEY[3].
//       5. Snapshot the timer value and stop the timer.
//       6. Call BinToDec() to convert number of cycles into Null-Terminated
//          ASCII string of decimal digits.
//       7. Call PutTerm to write to the terminal screen.
// Defn: ASCII_OFFSET 48 = Decimal value offset btw a Dec# and its ASCII value.
//       BASE_ADDR    0xDEADBEEF = Base address of JTAG UART.
//       WSPACE_MASK  0xFFFF0000 = Mask for the WSPACE bits of the cont. reg.
//       FOREVER      1  = Infinite Loop.
// Vars: juDataPtr = Pointer to data register of JTAG UART.
//       juCtrlPtr = Pointer to control register of JTAG UART.
//------------------------------------------------------------------------------
#include <alt_types.h>

# define ASCII_OFFSET 48         // Decimal value offset
# define BASE_ADDR    0xDEADBEEF // Base address of JTAG UART
# define WSPACE_MASK  0xFFFF0000 // Mask for the WSPACE bits of the cont. reg.
# define FOREVER      1          // Infinite Loop

volatile alt_u32 * juDataPtr = (alt_u32*)  BASE_ADDR;      // Ptr to data reg.
volatile alt_u32 * juCtrlPtr = (alt_u32*) (BASE_ADDR + 1); // Ptr to cont. reg.

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
      while ((*juCtrlPtr & WSPACE_MASK) == 0) {} // Wait for WSPACE > 0
      *juDataPtr = *strPtr;                      // Write char to terminal
      strPtr++;                                // Advance to next char
   }

   // When char = NULL
   while ((*juCtrlPtr & WSPACE_MASK) == 0) {}    // Wait for WSPACE > 0
   *juDataPtr = 0x0A;                            // Ouptut newline char (^LF)
}
//------------------------------------------------------------------------------

void BinToDec (alt_u32 * strPtr, alt_u32 num)
//------------------------------------------------------------------------------
// Func: Using a pointer to the first element of an ASCII Character string 
//       buffer in memory, and a 32-bit unsigned integer. Converts the 32-bit
//       unsigned integer into a decimal number of the same numerical value.
// Meth: As each decimal digit is calculated the digit is stored as an alt_u8
//       variable, such that the digit occupies the least significant nybble of
//       the whole byte. The decimal digit is then encoded into its ASCII
//       character, suitable for printing onscreen. Finally, that digit is
//       stored in the correct entry in the buffer whose pointer was passed in
//       the arguments. The character string is then null terminated and all
//       unused "leading" characters in the string are padded with the blank
//       "space" character.
// Args: strPtr = Pointer to first element in ASCII Character string buffer.
//       num    = Unsigned integer to be converted to an ASCII Character.
// Vars: digit  = Current digit of num being converted.
//------------------------------------------------------------------------------
{
    alt_u8 digit = 0;                 // Current digit of num being converted
    
    while (num != 0)                  // While there are still digits to read
    {
        digit = num % 10;             // Calculate & store next decimal digit
        num = num / 10;               // Remove least significant digit
        digit = digit + ASCII_OFFSET; // Encode digit to ASCII
        // Store digit to correct pos in buffer

        // Null-Terminate character string

        // Pad out leading characters with ' '
    }
}
//------------------------------------------------------------------------------