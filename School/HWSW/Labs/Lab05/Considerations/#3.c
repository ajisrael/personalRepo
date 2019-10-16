// ============================================================================
# include <alt_types.h>
# define SNAP  0x534E    // ASCII value of "SN"

volatile alt_u16 * timStatPtr = (alt_u16 *) TIME_ADDR;      // Ptr to stat. reg.
volatile alt_u16 * timSnpLPtr = (alt_u16*) (TIME_ADDR +16); // Ptr to snpL. reg.
volatile alt_u16 * timSnpHPtr = (alt_u16*) (TIME_ADDR +20); // Ptr to snpH. reg.

void main() {
	// -------------------------------------------------------------------------
	// Func: Determines if the timer rolled over and if so is capable of
	//       measuring time for over the period of the rollover.
	// Meth: If the timer rolled over, then write the maximum value to the lower 
	// 		 32-bits of an unsigned 64-bit integer. Then take a snapshot of the 
	//       timer and write that value to the upper 32-bits of the unsigned 
	//       64-bit integer. If the timer didn't roll over, then just clear the
	//       upper 32-bits, take a snapshot and save it to the lower 32-bits.
	// Vars: bit-64 = A union variable to imitate an unsigned 64-bit integer.
	// -------------------------------------------------------------------------
	union {
		alt_u32 H[2];
		alt_u16 Q[4]; 
	}bit64;             			  // Variable to act like 64-bit integer
	
   if ((*timStatPtr & 0x1))           // If there was a rollover
   {
		*timSnpLPtr  = SNAP;          // Trigger snap registers
		bit64.H[0] = 0xFFFFFFFF;      // Write the max value to the lower 32-bits
		bit64.Q[2] = *timSnapLPtr;    // Save the value in the snap registers
		bit64.Q[3] = *timSnapHPtr;    // to the upper 32-bits
   }
   else								  // Else
   {
		*timSnapLPtr = SNAP;		  // Trigger snap registers
		bit64.Q[0] = *timSnapLPtr;    // Save the value in the snap registers
		bit64.Q[1] = *timSnapHPtr;    // to the lower 32-bits
		bit64.H[1] = 0x00000000;      // Clear the upper 32-bits
   }
}
// ============================================================================
