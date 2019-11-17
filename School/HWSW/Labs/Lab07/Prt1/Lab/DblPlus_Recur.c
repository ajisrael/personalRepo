//------------------------------------------------------------------------------
// Prog: DblPlus_Recur.c
// Func: Test the recursive DblPlus function with different values of N.
//       Print the resulting run time and stack memory usage after each test.
//------------------------------------------------------------------------------
#include <alt_types.h>

# define FOREVER      1             // Infinite Loop
# define N_MIN 0                    // Minimum value for n
# define N_MAX 30                   // Maximum value for n
# define MAX_TIMER    0XFFFFFFFF    // Max value for timer
# define WSPACE_MASK  0xFFFF0000    // Mask for the WSPACE bits of the cont. reg.
# define ASCII_OFFSET 48            // Decimal value offset

# define JTUA_ADDR    0x00081068	// Base address of JTAG UART
# define TIME_ADDR    0x00081020    // Base address of timer
# define BUTN_ADDR    0x00081050    // Base address of pushbutton PIO

// Fun definitions to write to timer registers
# define GO           0x484F    // ASCII value of "GO"
# define CLEAR        0x434C    // ASCII value of "CL"
# define SNAP         0x534E    // ASCII value of "SN"

// Stack pointer variables for calculating stack memory usage
alt_u32 spInitial = 0;
alt_u32 spFinal = 0;

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
volatile alt_u32 * pbEdgePtr  = (alt_u32*) (BUTN_ADDR +12); // Ptr to edgc. reg.

void BinToDec (alt_u8 * strPtr, alt_u32 num)
//------------------------------------------------------------------------------
// Func: Using a pointer to the first element of an ASCII Character string 
//       buffer in memory, and a 32-bit unsigned integer. Converts the 32-bit
//       unsigned integer into a decimal number of the same numerical value.
// Args: strPtr = pointer to start of char string in memory
//       num    = the number of characters in the string
//------------------------------------------------------------------------------
{
    alt_u8 digit = 0;                 // Current digit of num being converted
    alt_u8 pos   = 10;                // Current position in strPtr

	if(num == 0) 
	{
		strPtr[pos] = '0';
		pos--;
	}
	
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
   //*juDataPtr = 0x0A;                            // Ouptut newline char (^LF)
}

alt_u32 NextN(alt_u32 n)
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
		if(n != N_MAX) 
		{
			return n + 1;
		}
		else
		{
			return -1;
		}	
    }
}

alt_u32 DblPlus(alt_u32 n)
{
// -----------------------------------------------------------------------------
// Func: Iteratively calculate the following recursive function.
//       DblPlus(n) = n + 2 * DblPlus(n) & DblPlus(0) = 1
// Args: n = value of n to be doubled
// -----------------------------------------------------------------------------
	asm("or %[RspFinal], sp, r0 \n\t"   // Read the stack pointer
		:[RspFinal] "=r" (spFinal)
		:
		: "memory"
		);

	alt_u32 result = 1;
	for(alt_u32 i = 1; i <= n; i++)
	{
		result = (i + 2*result);
	}
	
	return result;
}

void ExecuteTest()
// -----------------------------------------------------------------------------
// Func: Execute a test of DblPlus with values of n[0..30].
//       Print the results to the console.
// Args: None
// -----------------------------------------------------------------------------
{
	union // Holds value of timer
    {
        alt_u32 F;
        alt_u16 H[2];
    } stopVal;
	
	// Initialize string with all spaces and null terminator
	alt_u8 strN[12] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x00};
	alt_u8 strResult[12] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x00};
    alt_u8 strTimer[12] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x00};
	alt_u8 strMemory[12] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x00};
	
	PutTerm("          n        DblPlus(n)        Elapsed-Time        Stack Mem.\n");
	
	alt_u32 n = N_MIN;
	while(n != -1) 
	{
		asm("or %[RspInitial], sp, r0 \n\t"   // Read the stack pointer
			:[RspInitial] "=r" (spInitial)
			:
			:"memory"
		);
		
		*timPerLPtr = GO;                 // Reset timer
		*timCtrlPtr |= 0x4;               // Start timer
		alt_u32 result = (alt_u32) DblPlus(n);      // Run DblPlus Function

		// Snapshot the timer
		*timSnpLPtr  = SNAP;
        stopVal.H[0] = *timSnpLPtr;
        stopVal.H[1] = *timSnpHPtr;
		
		// Convert N and result to strings
		BinToDec(strN, n);
		BinToDec(strResult, result);
		
		// Calculate the change in time and convert to decimal
        stopVal.F = MAX_TIMER - stopVal.F - 17;
		BinToDec(strTimer, stopVal.F); 
		
		// Calc. memory usage and convert to decimal
		alt_u32 stackUsage = spInitial - spFinal;
		BinToDec(strMemory, stackUsage);
		
		// Print Statistics
		PutTerm("");
		PutTerm(strN);
		PutTerm("     ");
		PutTerm(strResult);
		PutTerm("     ");
		PutTerm(strTimer);
		PutTerm("     ");
		PutTerm(strMemory);
		PutTerm("\n");
		
		// Advance n
		n = NextN(n);
	}
	PutTerm("\n");
}


void main() 
// -----------------------------------------------------------------------------
// Func: Handles the pushbuttons that exit the program and execute another test.
// Args: None
// -----------------------------------------------------------------------------
{
	while(FOREVER) {
		// Wait for falling edge from KEY[1] or KEY[3]

        while (*pbEdgePtr == 0x0) {}
		
		// Exit program if KEY[1] was pressed
		if((*pbEdgePtr & 0x1) == 0x1)
		{
			break;
		}
		
		// Execute a test if KEY[3] was pressed
		if((*pbEdgePtr & 0x4) == 0x4) 
		{
			ExecuteTest();
			*pbEdgePtr = CLEAR;
		}
	}
	PutTerm("Terminating Program");
}
// -----------------------------------------------------------------------------