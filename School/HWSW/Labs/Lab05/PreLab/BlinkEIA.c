//------------------------------------------------------------------------------
// Revs: 2019.10.15 - Alex Israels
// Prog: BlinkEIA.c
// Func: Given a Nios Timer module in continous run mode with a fixed period,
//       Toggle all 18 DE2-115 LEDs every time the timer rolls over past zero.
// Defn: LEDS_BASE = LED PIO Base address.
//       TIMR_BASE = Timer Base address.
// Vars: ledPtr = Pointer to 32-bit register for base address of LED PIO.
//       timPtr = Pointer to 16-bit register for base address of timer.
//------------------------------------------------------------------------------
#include <alt_types.h>
#define  LEDS_BASE 0xDEADBEEF       // LED PIO Base address. Value TBD by Qsys.
#define  TIMR_BASE 0xBEADBEEF       // TIMER   Base address. Value TBD by Qsys. 

void main ()
{
  volatile alt_u32 * ledPtr = (alt_u32 *) LEDS_BASE;    // init ptr to LEDs
  volatile alt_u16 * timPtr = (alt_u16 *) TIMR_BASE;    // init ptr to timr

  *ledPtr = 0x00000000;               // Set all LEDs to OFF
  *(timPtr + 1) |= 0x6;               // Set timr's CONT bit to continuous mode
                                      // & set the START bit to start timr 

  while (1)                           // Do forever
  { 
    volatile asm(
        "SPIN:                          \n\t" // Spin while TO bit = 0
        "   orhi   r8, %[Rtim], 0x0000  \n\t" // Load addr of timer
        "   ori    r8, r8,      0x0000  \n\t"
        "   ldbuio r8, 0(r8)            \n\t" // Load first byte of status reg.
        "   andi   r8, r8, 0x01         \n\t" // Mask out TO bit
        "   beq    r8, r0, SPIN         \n\t" // Spin if TO bit = 0
        "orhi  r9, %[Rled], 0x0000      \n\t" // Load addr of led PIO
        "ori   r9, r9,      0x0000      \n\t" 
        "ldwio r9, 0(r9)                \n\t" // Load state of LEDs
        "xorhi r9, r9, 0xFFFF           \n\t" // Toggle all LEDs
        "xori  r9, r9, 0xFFFF           \n\t"
        "andi  r8, r8, 0x00             \n\t" // Clear timr TO bit
        : // No outputs
        : [Rtim] "r" (timPtr), [Rled] "r" (ledPtr)
        : "r8", "r9"
    );
  }
}
//------------------------------------------------------------------------------