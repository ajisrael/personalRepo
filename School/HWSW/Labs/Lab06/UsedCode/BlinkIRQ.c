//------------------------------------------------------------------------------
// Revs: 2019.10.19 - Logan Wilkerson
// Prog: BlinkIRQ.c
// Func: Given a Nios Timer module in continous run mode with a fixed period,
//       Toggle all 18 DE2-115 LEDs every time the timer rolls over past zero.
//       Utilizes IRQs to interrupt the CPU instead of a polling loop.
//------------------------------------------------------------------------------
#include <alt_types.h>
#include "GenericExcepHndlr.c"  // Needed by Nios C compiler - DO NOT EDIT!

#define  FOREVER      1         // Infinite Loop
#define  LEDS_BASE 0x00081080   // LED PIO Base address. Value TBD by Qsys.
#define  TIMR_BASE 0x00081020   // TIMER   Base address. Value TBD by Qsys. 

// Function prototypes to make sure they compile
void ResetHandler (void);
void ExcepHandler (void);
void IsrTimer     (void);

// Initialize base points to LEDs (32-bit) and Timer (16-bit)
volatile alt_u32 * ledPtr = (alt_u32 *) LEDS_BASE;    // init ptr to LEDs
volatile alt_u16 * timPtr = (alt_u16 *) TIMR_BASE;    // init ptr to timr

// -----------------------------------------------------------------------------
// IRQ Source Parser for all external interrupts. 
// Func: Examine reg ctrl4 (ipending) to determine which device caused the IRQ,
//       then call the correct ISR to service that device. 
// -----------------------------------------------------------------------------
void IrqParser(void)             // called by global exception handler
{
  alt_u32 irqPend;
  irqPend = __builtin_rdctl(4);  // Read ipending reg (ctl4) into a C variable
  if (irqPend & 0x4)             // If ipending reg [bit 1] = 1,9
     IsrTimer();                 // then call Pushbutton ISR to service the IRQ

  return;                        // Return to global exception handler
}

// -----------------------------------------------------------------------------
// Application dependent Interrupt Service Routine (ISR) for Timer IRQ.
// Func: Toggle all LEDs when a Timer interrupt occurs due to a timeout. 
// -----------------------------------------------------------------------------
void IsrTimer(void)              // called by IRQ Source Parser (IrqParser)
{
  *ledPtr ^= 0xFFFFFFFF;         // Toggle all LEDs
  *timPtr = 0xFFFE;              // Clear timer TO bit
  
  return;
}

void main ()
{
  alt_u32 tmpReg;                // 32-bit temporary variable
	
  // Globally enable exceptions and IRQs
  tmpReg = __builtin_rdctl(0) | 0x5;
  __builtin_wrctl(0, tmpReg);
  
  // Enable timer IRQs
  tmpReg = __builtin_rdctl(3) | 0x4;
  __builtin_wrctl(3, tmpReg);

  *ledPtr = 0x00000000;          // Initialize all LEDs to OFF
  *(timPtr + 2) |= 0x07;         // Set timer to CONT and enable ITO
                                 // & set the START bit to start timer

  while (FOREVER) {}             // Wait for an IRQ to occur...
}
// ========================================================================= EOF