//------------------------------------------------------------------------------
// Revs: 2019.10.14 - Alex Israels
// Revs: 2019.10.15 - Alex Israels - Logan Wilkerson
// Prog: BlinkIRQ.c
// Func: Given a Nios Timer module in continous run mode with a fixed period,
//       Toggle all 18 DE2-115 LEDs every time the timer rolls over past zero 
//       using an ISR
// Defn: LEDS_BASE = LED PIO Base address.
//       TIMR_BASE = Timer Base address.
// Vars: ledPtr = Pointer to 32-bit register for base address of LED PIO.
//       timPtr = Pointer to 16-bit register for base address of timer.
//------------------------------------------------------------------------------
#include <alt_types.h>

#define  LEDS_BASE 0x00081070       // LED PIO Base address. Value TBD by Qsys.
#define  TIMR_BASE 0x00081020       // TIMER   Base address. Value TBD by Qsys.

volatile alt_u32 * ledPtr = (alt_u32 *) LEDS_BASE;    // init ptr to LEDs
volatile alt_u16 * timPtr = (alt_u16 *) TIMR_BASE;    // init ptr to timr

void IrqParser(void)             // called by global exception handler
// -----------------------------------------------------------------------------
// IRQ Source Parser for all external interrupts. 
// Func: Examine reg ctrl4 (ipending) to determine which device caused the IRQ,
//       then call the correct ISR to service that device. 
// Note: 1. In this example application, there is ONLY the Timer Overflow ISR, 
//          but others can easily be edited into this function for other apps.
//       2. This example ASSUMES pushbutton IRQ is mapped to bit 1 of ctrl regs
//          ctl3 & ctl4. The ACTUAL mapping will be done by QSYS.
// -----------------------------------------------------------------------------
{
    alt_u32 irqPend;
    irqPend = __builtin_rdctl(4);  // Read ipending reg (ctl4) into a C variable
    if (irqPend & 0x2)             // If ipending reg [bit 1] = 1,
        IsrTimerOver();            // then call Timer ISR to service the IRQ

    return;                        // Return to global exception handler
}


void IsrTimerOver(void)           // called by IRQ Source Parser (IrqParser)
// -----------------------------------------------------------------------------
// Application dependent Interrupt Service Routine (ISR) for Timer Overflow IRQ.
// Func: Toggles the LEDs on and off and clears the timer overflow bit.
// -----------------------------------------------------------------------------
{
    *ledPtr ^= 0xFFFFFFFF;               // Toggle all LEDs     
    *timPtr = 0xFFFE;                    // Clear timr TO bit
    return;
}

void main ()
{
    // Enable IRQs
    __builtin_wrctl(0, 5); // Write 101 to status reg to enable all excps & IRQs
    __builtin_wrctl(3, 1); // Write 1   to ienable reg to enable buttons IRQ

    *ledPtr = 0x00000000;               // Set all LEDs to OFF
    *(timPtr + 2) |= 0x06;              // Set timr's CONT bit to continuous mode
                                        // & set the START bit to start timr 

    while (1) {}                        // Sleep
}
//------------------------------------------------------------------------------