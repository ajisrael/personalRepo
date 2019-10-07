// ========================================================================== BOF
// Orig: 2019.10.01 - Alex Israels
// Func: Increases a counter infinitely and "writes" it to the PIO address 
//       provided by Quartus. (Need to manually enter the PIO Address.)
// Args: pioAddr = Address of PIO to drive 7-segment display
//		   counter = Value to be written to PIO
// ------------------------------------------------------------------------------

#include <alt_types.h>

void main()
// ------------------------------------------------------------------------------
// Func: Initializes memory address of PIO. Enters infinite loop and increases
//       a counter by 1 each loop and writes its value to the PIO address.
// Args: pioAddr = Address of PIO to drive 7-segment display
//       counter = Value to be written to PIO
// ------------------------------------------------------------------------------
{
	volatile altu_32 * pioAddr = 0xdeadbeef; // ADDR of PIO
	         altu_32   counter = 0;          // Counter value to be written to PIO

	while (0 == 0)
	{
		counter = counter + 1;
		*(pioAddr) = counter;
	}
}

// ========================================================================== EOF