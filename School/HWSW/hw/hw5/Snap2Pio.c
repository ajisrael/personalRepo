// ------------------------------------------------------------------------------
// Orig: 2019.10.14 - Alex Israels
// Func: Recieves two arguments, timPtr and pioPtr. Declares a union and 
//       initiates a snapshot of the timer value. Saves the snapshot value to
//       union and ouputs the value to the 32-bit Nios-II PIO assuming al pins of
//       the PIO were previously set to the output direction.
// Args: timPtr  = A type (alt_u16 *) pointer to the base address of the timer
//       pioPtr  = A type (alt_u32 *) pointer to the base address of a 32-bit PIO
// Vars: timSnap = Union for reading in lower and uppper 16 bit registers of snap
// ------------------------------------------------------------------------------

#include <alt_types.h>

void Snap2Pio (volatile alt_u16 * timPtr, volatile alt_u32 * pioPtr)
{
   union
   {
      alt_u32 F;      // Full word
      alt_u16 H[2];   // Two half words
   } timSnap;

   // Trigger snapshot
   *(timPtr + 4) = 0x1;

   // Read snapshot
   timSnap.H[0] = *(timPtr + 4);
   timSnap.H[1] = *(timPtr + 5);

   // Store to PIO
   * pioPtr = timSnap.F;
}