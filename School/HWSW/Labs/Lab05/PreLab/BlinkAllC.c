//------------------------------------------------------------------------------
// Revs: 2019.10.14 - Alex Israels
// Prog: BlinkAllC.c
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

  while (1)                              // Do forever
  { 
    while(~(*timPtr & 0x1)) {}           // spin while TO bit=0 (=> no rollover)
    *ledPtr ^= 0xFFFFFFFF;               // Toggle all LEDs     
    *timPtr &= 0xFFFE;                   // Clear timr TO bit
  }
}
//------------------------------------------------------------------------------