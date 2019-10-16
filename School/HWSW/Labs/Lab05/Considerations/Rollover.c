// ============================================================================
# include <alt_types.h>
# define TIME_ADDR 0xDEADBEEF // Base address of the timer

void checkRollover()
// ----------------------------------------------------------------------------
// Func: Determines if the timer rolled over.
// Meth: If the timer rolled over then the lsb of the status register is 
//       set to a 1. To determine if the timer rolled over the function
//       checks the value of this bit and writes "Rollover!" to the console.
// Vars: timStatPtr = Pointer to the status register of the timer.
//       str        = String to hold "Rollover!"
// ----------------------------------------------------------------------------
{
   volatile alt_u16 * timStatPtr = (alt_u16 *) TIME_ADDR; // Ptr to stat. reg.
   alt_u8 str[10];                                        // String to user

   if ((*timStatPtr & 0x1))      // If rollover
   {
      *str   = "Rollover!"       // Tell user Rollover!
      str[9] = 0x00;
      PutTerm(str);
   }
} 
// ============================================================================
