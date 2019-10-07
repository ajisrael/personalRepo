// ========================================================================== BOF
// Orig: 2019.09.21 - Alex Israels
// Rev : 2019.09.21 - Alex Israels
// Func: Increments through a list of values to find the smallest one and
//       stores it in a variable.
// Args: N 		 	 = Number of entries in the list
// 		entries 	 = List of all entries
//		 	i 			 = Increment variable for traversing entries
//		 	currVal 	 = The current value of the entry to be compared
// 		currMinVal  = The current minimum value of the entries (in loop)
// 		finalMinVal = The final minimum value of the entries (loop complete)
// ------------------------------------------------------------------------------

#include <alt_types.h>

alt_u8 main()
{
	alt_u8 N = 7;                               // Number of entries in list
	alt_u8 entries[] = { 4, 5, 3, 6, 9, 8, 2 }; // Values of entries in list
	alt_u8 i = 0;								// Ref for elements in entries
	alt_u8 currVal = -1;						// Value of current entry in 
												// entries
	alt_u8 currMinVal = 255;					// Value of smallest current entry
	alt_u8 finalMinVal = -1;					// Value of final smallest entry

	for (i < N; i++)					// Increment i to get next entry
	{	
		currVal = entries[i];			// Set currVal to the next entry 
		if (currVal < currMinVal)		// Check if currVal is lt currMinVal
		{								// If it is ...
			currMinVal = currVal;		// Set currMinVal to currVal
		}								// Else ...
	}									// go back to top of loop

	finalMinVal = currMinVal;			// Save currMinVal to memory
}