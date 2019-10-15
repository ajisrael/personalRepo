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
// Defn: FOREVER      1  = Infinite Loop.
//       ASCII_OFFSET 48 = Decimal value offset btw a Dec# and its ASCII value.
//       JTUA_ADDR    0xDEADBEEF = Base address of JTAG UART.
//       WSPACE_MASK  0xFFFF0000 = Mask for the WSPACE bits of the cont. reg.
//       TIME_ADDR    0xDEADBEAD = Base address of timer.
//       BUTN_ADDR    0xDEADFEED = Base address of pushbutton PIO.
//       KEY3_MASK    0x00000004 = Mask for KEY[3] pin
//       MAX_TIMER    0xFFFFFFFF = Max value for timer in Nios.
// Vars: juDataPtr  = Pointer to data register of JTAG UART.
//       juCtrlPtr  = Pointer to control register of JTAG UART.
//       timStatPtr = Pointer to status register of timer.
//       timCtrlPtr = Pointer to control register of timer.
//       timPerLPtr = Pointer to periodl register of timer.
//       timPerHPtr = Pointer to periodh register of timer.
//       timSnpLPtr = Pointer to snapl register of timer.
//       timSnpHPtr = Pointer to snaph register of timer.
//       pbDircPtr  = Pointer to direction register of pushbutton PIO.
//       pbEdgePtr  = Pointer to edge capture register of pushbutton PIO.
//------------------------------------------------------------------------------
#include <alt_types.h>

# define FOREVER      1          // Infinite Loop
# define ASCII_OFFSET 48         // Decimal value offset
# define JTUA_ADDR    0xDEADBEEF // Base address of JTAG UART
# define WSPACE_MASK  0xFFFF0000 // Mask for the WSPACE bits of the cont. reg.
# define TIME_ADDR    0xDEADBEAD // Base address of timer
# define BUTN_ADDR    0xDEADFEED // Base address of pushbutton PIO
# define KEY3_MASK    0x00000004 // Mask for KEY[3] pin
# define MAX_TIMER    0XFFFFFFFF // Max value for timer

// Initialize registers to JTAG UART: Registers are 32-bits, offset = 4 bytes
volatile alt_u32 * juDataPtr  = (alt_u32*)  JTUA_ADDR;      // Ptr to data  reg.
volatile alt_u32 * juCtrlPtr  = (alt_u32*) (JTUA_ADDR + 4); // Ptr to ctrl. reg.

// Initialize registers to Timer: Registers are 16-bits, offset = 2 bytes
volatile alt_u16 * timStatPtr = (alt_u16*)  TIME_ADDR;      // Ptr to stat  reg.
volatile alt_u16 * timCtrlPtr = (alt_u16*) (TIME_ADDR + 2); // Ptr to ctrl. reg. 
volatile alt_u16 * timPerLPtr = (alt_u16*) (TIME_ADDR + 4); // Ptr to perL. reg.
volatile alt_u16 * timPerHPtr = (alt_u16*) (TIME_ADDR + 6); // Ptr to perH. reg.
volatile alt_u16 * timSnpLPtr = (alt_u16*) (TIME_ADDR + 8); // Ptr to snpL. reg.
volatile alt_u16 * timSnpHPtr = (alt_u16*) (TIME_ADDR +10); // Ptr to snpH. reg.

// Initialize registers to PushButton PIO: 32-bit reg., offset = 4 bytes
volatile alt_u32 * pbDircPtr  = (alt_u32*) (BUTN_ADDR + 4); // Ptr to dirc. reg.
volatile alt_u32 * pbEdgePtr  = (alt_u32*) (BUTN_ADDR +12); // Ptr to edgc. reg.

// To be explicit initialize direction of all pins to input on PB PIO:
*pbDircPtr = 0x00000000;

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
      strPtr++;                                  // Advance to next char
   }

   // When char = NULL
   while ((*juCtrlPtr & WSPACE_MASK) == 0) {}    // Wait for WSPACE > 0
   *juDataPtr = 0x0A;                            // Ouptut newline char (^LF)
}
//------------------------------------------------------------------------------

void main()
// -----------------------------------------------------------------------------
// Func: Print the argument pointer's char string to JTAG UART using pointer
//       arithematic.
// Vars: stopVal = Value of timer when stopped from seccond KEY3 press.
// -----------------------------------------------------------------------------
{
    union
    {
        alt_u32 F;
        alt_u16 H[2];
    } stopVal;
    
    while (FOREVER)
    {
        // Initialize timer (0x484F = "GO")
        *timPerLPtr = 0x484F;

        // Wait for falling edge from KEY[3]     
        while ((*pbEdgePtr & KEY3_MASK) != 1) {}

        // Start the timer (0x4 sets start bit to 1)
        *timCtrlPtr |= 0x4;

        // Clear edge register (0x434C454152 = "CLEAR")
        *pbEdgePtr = 0x434C454152;

        // Wait for next falling edge from KEY[3]     
        while ((*pbEdgePtr & KEY3_MASK) != 1) {}

        // Clear edge register (0x434C454152 = "CLEAR")
        *pbEdgePtr = 0x434C454152;

        // Snapshot the timer (0x534E4150 = "SNAP")
        *timSnpLPtr  = 0x534E4150;
        stopVal.H[0] = *timSnpLPtr;
        stopVal.H[1] = *timSnpHPtr;

        // Stop the timer (0x8 sets stop bit to 1)
        *timCtrlPtr |= 0x8;
        
        // Calculate the change in time
        stopVal.F = MAX_TIMER - stopVal.F;

        // Convert time change to string
        BinToDec(str, stopVal.F);

        // Print to terminal
        PutTerm(str);
    }
}