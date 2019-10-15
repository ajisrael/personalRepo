// ------------------------------------------------------------------------------
// Orig: 2019.10.14 - Alex Israels
// Func: Recieves two arguments, timPtr and pioPtr. Declares a union and 
//       initiates a snapshot of the timer value. Saves the snapshot value to
//       union and ouputs the value to the 32-bit Nios-II PIO assuming al pins of
//       the PIO were previously set to the output direction.
// Args: timPtr  = A type (alt_u16 *) pointer to the base address of the timer
//       pioPtr  = A type (alt_u32 *) pointer to the base address of a 32-bit PIO
// ------------------------------------------------------------------------------

#include <alt_types.h>

void Snap2Pio (volatile alt_u16 * timPtr, volatile alt_u32 * pioPtr)
{
   // Begin EIA
   volatile asm
   (
      "ldhuio r8, 0(%[Rtim])    \n\t" // r8 = ADDR of timPtr
      "addi   r9, r0, 0x1       \n\t" // r9 = 1
      "stbio  8(r8), 0(r9)      \n\t" // Write val to snap to trigger snap
      "sthuio 0(%[Rpio]), 8(r8) \n\t" // store snapl to lower half of PIO
      "sthuio 2(%[Rpio]), 10(r8)\n\t" // store snaph to upper half of PIO
      :[Rpio] "=r" (pioPtr)
      :[Rtim] " r" (timPtr)
      :"r8", "r9", "memory"
   );
}