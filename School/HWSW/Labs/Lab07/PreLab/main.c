//------------------------------------------------------------------------------
// Orig: 2019.10.15 - Alex Israels
// Revs: 2019.10.15 - Alex Israels - Logan Wilkerson
// Prog: ButtonTimISR.c
// Func: Calculates the number of clock cycles between button presses.
// Asum: A Nios system with an input pIO for the four DE2 pushbuttons, and a
//       fixed period timer (i.e., Qsys defined the period, with the "Writable
//       Period" option set to OFF).
// Meth: Uses an IRQ when Key 3 is pressed. On the first press it starts the
//       timer. On the second it calculates the difference between the max value
//       of the timer and the current value, stops the timer, and writes the
//       time out to the terminal.  
// Defn: FOREVER      1  = Infinite Loop.
//       ASCII_OFFSET 48 = Decimal value offset btw a Dec# and its ASCII value.
//       JTUA_ADDR    0xDEADBEEF   = Base address of JTAG UART.
//       WSPACE_MASK  0xFFFF0000   = Mask for the WSPACE bits of the cont. reg.
//       TIME_ADDR    0xDEADBEAD   = Base address of timer.
//       BUTN_ADDR    0xDEADFEED   = Base address of pushbutton PIO.
//       KEY3_MASK    0x00000001   = Mask for KEY[3] pin.
//       MAX_TIMER    0xFFFFFFFF   = Max value for timer in Nios.
//       GO           0x484F       = ASCII value of "GO"
//       CLEAR        0x434C       = ASCII value of "CL"
//       SNAP         0x534E       = ASCII value of "SN"
// Vars: juDataPtr  = Pointer to data register of JTAG UART.
//       juCtrlPtr  = Pointer to control register of JTAG UART.
//       timStatPtr = Pointer to status register of timer.
//       timCtrlPtr = Pointer to control register of timer.
//       timPerLPtr = Pointer to periodl register of timer.
//       timPerHPtr = Pointer to periodh register of timer.
//       timSnpLPtr = Pointer to snapl register of timer.
//       timSnpHPtr = Pointer to snaph register of timer.
//       pbDircPtr  = Pointer to direction register of pushbutton PIO.
//       pbInMsPtr  = Pointer to inturrupt mask resgister of pushbutton PIO.
//       pbEdgePtr  = Pointer to edge capture register of pushbutton PIO.
//       stopVal    = Union variable to hold timer delay.
//------------------------------------------------------------------------------
#include <alt_types.h>
#include "GenericExcepHndlr.c"      // Needed by Nios C compiler - DO NOT EDIT!

# define FOREVER      1             // Infinite Loop
# define ASCII_OFFSET 48            // Decimal value offset
# define JTUA_ADDR    0x000810A8	// Base address of JTAG UART
# define WSPACE_MASK  0xFFFF0000    // Mask for the WSPACE bits of the cont. reg.
# define TIME_ADDR    0x00081020    // Base address of timer
# define BUTN_ADDR    0x00081090    // Base address of pushbutton PIO
# define KEY3_MASK    0x00000001    // Mask for KEY[3] pin
# define MAX_TIMER    0XFFFFFFFF    // Max value for timer

// Fun definitions to write to timer registers
# define GO           0x484F    // ASCII value of "GO"
# define CLEAR        0x434C    // ASCII value of "CL"
# define SNAP         0x534E    // ASCII value of "SN"

// Definitions for prelab algorithims
# define N_MIN  0 // Minimum value of n
# define N_MAX 30 // Maximum value of n 

// Global variables for lab 7
alt_u32 n = 0;                // Number for DblPlus fn.
alt_u32 nPlus = 0;            // Result of DblPlus fn.

// Initialize registers to JTAG UART: Registers are 32-bits, offset = 4 bytes
volatile alt_u32 * juDataPtr  = (alt_u32*)  JTUA_ADDR;      // Ptr to data  reg.
volatile alt_u32 * juCtrlPtr  = (alt_u32*) (JTUA_ADDR + 4); // Ptr to ctrl. reg.

// Initialize registers to Timer: Registers are 16-bits, offset = 2 bytes
volatile alt_u16 * timStatPtr = (alt_u16*)  TIME_ADDR;      // Ptr to stat  reg.
volatile alt_u16 * timCtrlPtr = (alt_u16*) (TIME_ADDR + 4); // Ptr to ctrl. reg. 
volatile alt_u16 * timPerLPtr = (alt_u16*) (TIME_ADDR + 8); // Ptr to perL. reg.
volatile alt_u16 * timPerHPtr = (alt_u16*) (TIME_ADDR +12); // Ptr to perH. reg.
volatile alt_u16 * timSnpLPtr = (alt_u16*) (TIME_ADDR +16); // Ptr to snpL. reg.
volatile alt_u16 * timSnpHPtr = (alt_u16*) (TIME_ADDR +20); // Ptr to snpH. reg.

// Initialize registers to PushButton PIO: 32-bit reg., offset = 4 bytes
volatile alt_u32 * pbDircPtr  = (alt_u32*) (BUTN_ADDR + 4); // Ptr to dirc. reg.
volatile alt_u32 * pbInMsPtr  = (alt_u32*) (BUTN_ADDR + 8); // Ptr to irq mask
volatile alt_u32 * pbEdgePtr  = (alt_u32*) (BUTN_ADDR +12); // Ptr to edgc. reg.

// Global variable for timer value
union // Holds value of timer when stopped by KEY3
    {
        alt_u32 F;
        alt_u16 H[2];
    } stopVal;

void BinToDec (alt_u8 * strPtr, alt_u32 num)
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
//       pos    = Current position in strPtr. Set to 10 b/c strPtr will always
//                point to a 12 element null terminated array.
//       i      = Index variable for padding space characters.
//------------------------------------------------------------------------------
{
    alt_u8 digit = 0;                 // Current digit of num being converted
    alt_u8 pos   = 10;                // Current position in strPtr

    while (num != 0)                  // While there are still digits to read
    {
        digit = num % 10;             // Calculate & store next decimal digit
        num = num / 10;               // Remove least significant digit
        digit = digit + ASCII_OFFSET; // Encode digit to ASCII
        strPtr[pos] = digit;          // Store digit to correct pos in buffer
        pos--;						  // Decrement the index of the array
    }

    for (int i = 0; i <= pos; i++)    // Loop over remaining characters
    {
        strPtr[i] = ' ';              // Pad leading chars with ' '
    }

    strPtr[11] = 0x00;                // Null-Terminate character string
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

alt_u32 nNext(alt_u32 n)
// -----------------------------------------------------------------------------
// Func: Gets the next value of n for the main loop.
// Args: n = value to increment.
// -----------------------------------------------------------------------------
{
    if (n < 22)
    {
        return n + 5;
    }
    else
    {
        return n + 1;
    }
}

void mainLoop(void)
// -----------------------------------------------------------------------------
// Func: Resets and starts the timer.
//       Calculates DblPlus(n). Snapshots the timer. Calculates the elapsed time
//       and stack usage. Finally, outputs the data to the screen. Checks if
//       n equals N_MAX and either waits for a button edge or moves to next n.
// -----------------------------------------------------------------------------
{
    // Initialize timer (0x484F = "GO")
    *timPerLPtr = GO;
    
    // Start the timer (0x4 sets start bit to 1)
    *timCtrlPtr |= 0x4;

    nPlus = DblPlus(n, spMin);

    // Snapshot the timer (0x534E4150 = "SNAP")
    *timSnpLPtr  = SNAP;
    stopVal.H[0] = *timSnpLPtr;
    stopVal.H[1] = *timSnpHPtr;

    // Stop the timer (0x8 sets stop bit to 1)
    *timCtrlPtr |= 0x8;
    
    // Calculate the change in time
    stopVal.F = MAX_TIMER - stopVal.F;

    // Calculate stack usage
    spUsage = *spInit - *spMin;

    // Convert n to string and print to terminal
    BinToDec(nVal, n);
    PutTerm(nVal);

    // Convert nPlus to string and print to terminal
    BinToDec(dVal, nPlus);
    PutTerm(dVal);

    // Convert time change to string and print to terminal
    BinToDec(time, stopVal.F);
    PutTerm(time);

    // Convert spUsage to string and print to terminal
    BinToDec(stak, spUsage);
    PutTerm(stak);

    // Check for base case
    if (n == N_MAX)
    {
        return;
    }
    else
    {
        n = nNext(n);
        mainLoop();
    }
}

void IsrKey3(void)           // called by IRQ Source Parser (IrqParser)
// -----------------------------------------------------------------------------
// Application dependent Interrupt Service Routine (ISR) for Pushbutton IRQ.
// Func: Initializes the users screen with headers and sets n = N_MIN. Then
//       saves the initial Stack Pointer value. Then calls the main loop.
// Vars: str     = String to hold decimal values converted to ASCII
// -----------------------------------------------------------------------------
{
    // Initialize string with all spaces and null terminator: 
    // 2^32 = 11 digits max so str size is 12 for null terminator.
    alt_u8 nVal[12] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','\t'};
    alt_u8 dVal[12] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','\t'};
    alt_u8 time[12] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '.'\t'};
    alt_u8 stak[12] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x00};

    alt_u8 header[45] = "N\tDblPlus(n)\tElapsed-Time\tStack Memory Used" + 0x00; // Headers
    
    alt_u32 * spInit = NULL;      // Initial stack ptr value
    alt_u32 * spMin  = NULL;      // Minimum stack ptr value
    alt_u32 spUsage  = 0;         // Stack usage

    // Write column headers to console
    PutTerm(header);
    
    *(pbEdgePtr) = 0;           // Clear IRQ signal by writing to the
                                // edgecapture reg of the pushbutton PIO

    // Initialize n = N_MIN
    n = N_MIN;

    // Save SP value
    asm volatile
    (
        "stw r27, 0(%[Rinit]) \n\t"
        :[Rinit] "=r" (spInit)
        :
        :"memory"
    );

    mainLoop();
}

void IsrKey1(void)
{
    alt_u8 str[5] = "Done"+0x00;
    PutTerm(str);
}

void IrqParser(void)             // called by global exception handler
// -----------------------------------------------------------------------------
// IRQ Source Parser for all external interrupts. 
// Func: Examine reg ctrl4 (ipending) to determine which device caused the IRQ,
//       then call the correct ISR to service that device. 
// Note: 1. In this example application, there is ONLY the Pushbutton ISR, 
//          but others can easily be edited into this function for other apps.
//       2. This example ASSUMES pushbutton IRQ is mapped to bit 1 of ctrl regs
//          ctl3 & ctl4. The ACTUAL mapping will be done by QSYS.
// -----------------------------------------------------------------------------
{
    alt_u32 irqPend;
    irqPend = __builtin_rdctl(4);  // Read ipending reg (ctl4) into a C variable
    if (irqPend & 0x1)             // If ipending reg [bit 1] = 1,
        IsrKey3();                 // then call KEY3 ISR to service the IRQ
    else if (irqPend & 0x2)        // If ipending reg [bit 2] = 1
        IsrKey1();                 // then call KEY1 ISR to service the IRQ

    return;                        // Return to global exception handler
}

void main()
// -----------------------------------------------------------------------------
// Func: Print the argument pointer's char string to JTAG UART using pointer
//       arithematic.
// Vars: stopVal = Value of timer when stopped from seccond KEY3 press.
//       
// -----------------------------------------------------------------------------
{
    // Enable IRQs
    __builtin_wrctl(0, 5); // Write 101 to status reg to enable all excps & IRQs
    __builtin_wrctl(3, 1); // Write 10  to ienable reg to enable buttons IRQ

    * (pbInMsPtr) = 0xA;   // Enable button 3 and 1 to cause an IRQ
    
    while (FOREVER) {}     // Sleep
}
// -----------------------------------------------------------------------------