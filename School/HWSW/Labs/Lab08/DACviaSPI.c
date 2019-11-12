// ------------------------------------------------------------------------------
// Orig: 2019.11.12 - Alex Israels
// Prog: DACviaSPI.c
// Func: Interfaces with a DAC over SPI to genrate a sawtooth waveform.
// Asum: A fixed period timer (i.e., Qsys defined the period, with the "Writable
//       Period" option set to OFF).
// Meth: Uses an IRQ when timer rolls over that increments the value written to 
//       the DAC.
// Defn: FOREVER      1  = Infinite Loop.
//       TIME_ADDR    0xDEADBEAD   = Base address of timer.
//       TRDY         0x20         = Mask for TRDY bit.
// Vars: timStatPtr = Pointer to status register of timer.
//       timCtrlPtr = Pointer to control register of timer.
//       timPerLPtr = Pointer to periodl register of timer.
//       timPerHPtr = Pointer to periodh register of timer.
//       timSnpLPtr = Pointer to snapl register of timer.
//       timSnpHPtr = Pointer to snaph register of timer.
//       spiRxDataPtr = Pointer to rxdata reg of SPI.
//       spiTxDataPtr = Pointer to txdata reg of SPI.
//       spiStatusPtr = Pointer to status reg of SPI.
//       spiCtrlPtr   = Pointer to control reg of SPI.
//       spiSlvSelPtr = Pointer to slave select reg of SPI.
//------------------------------------------------------------------------------
#include <alt_types.h>
#include "GenericExcepHndlr.c"      // Needed by Nios C compiler - DO NOT EDIT!

# define FOREVER      1             // Infinite Loop
# define TIME_ADDR    0x00081020    // Base address of timer
# define TRDY         0x20          // Mask for TRDY bit

// Initialize registers to Timer: Registers are 16-bits, offset = 2 bytes
volatile alt_u16 * timStatPtr = (alt_u16*)  TIME_ADDR;      // Ptr to stat  reg.
volatile alt_u16 * timCtrlPtr = (alt_u16*) (TIME_ADDR + 4); // Ptr to ctrl. reg. 
volatile alt_u16 * timPerLPtr = (alt_u16*) (TIME_ADDR + 8); // Ptr to perL. reg.
volatile alt_u16 * timPerHPtr = (alt_u16*) (TIME_ADDR +12); // Ptr to perH. reg.
volatile alt_u16 * timSnpLPtr = (alt_u16*) (TIME_ADDR +16); // Ptr to snpL. reg.
volatile alt_u16 * timSnpHPtr = (alt_u16*) (TIME_ADDR +20); // Ptr to snpH. reg.

// Initialize registers to SPI:
volatile alt_u32 * spiRxDataPtr = (alt_u32*)  SPI_ADDR;      // Ptr @ rxdata reg
volatile alt_u32 * spiTxDataPtr = (alt_u32*) (SPI_ADDR + 4); // Ptr @ txdata reg
volatile alt_u32 * spiStatusPtr = (alt_u32*) (SPI_ADDR + 8); // Ptr @ status reg
volatile alt_u32 * spiCtrlPtr   = (alt_u32*) (SPI_ADDR +12); // Ptr @ ctrl   reg
volatile alt_u32 * spiSlvSelPtr = (alt_u32*) (SPI_ADDR +20); // Ptr @ slave sel

// Global variable for DAC value
volatile alt_u16 DACVal = 0;

void IsrTimer(void)           // called by IRQ Source Parser (IrqParser)
// -----------------------------------------------------------------------------
// Application dependent Interrupt Service Routine (ISR) for Timer IRQ.
// Func: Increments the value written to DAC to generate sawtooth wave.
// Vars: maskedStatus = Temporary variable to determine if TRDY bit is set.
// -----------------------------------------------------------------------------
{
    alt_u32 maskedStatus = 0; // Temp variable for masked status reg value

    timStatPtr &= 0xFFFE      // Clear TO bit

    if (DACVal == 0xFFF)      // Check for max val
    {
        DACVal = 0;           // Reset to zero
    }
    else
    {
        DACVal++;             // Otherwise increment by 1
    }

    do
    {
    maskedStatus = *spiStatusPtr & TRDY; // Check for TRDY
    }
    while (maskedStatus != TRDY)         // Wait for TRDY

    * spiTxDataPtr = DACVal;             // Write new val to DAC

    return // Return to main
}

void IrqParser(void)             // called by global exception handler
// -----------------------------------------------------------------------------
// IRQ Source Parser for all external interrupts. 
// Func: Examine reg ctrl4 (ipending) to determine which device caused the IRQ,
//       then call the correct ISR to service that device.
// -----------------------------------------------------------------------------
{
    alt_u32 irqPend;
    irqPend = __builtin_rdctl(4);  // Read ipending reg (ctl4) into a C variable
    if (irqPend & 0x1)             // If ipending reg [bit 1] = 1,
        IsrTimer();               // then call Pushbutton ISR to service the IRQ

    return;                        // Return to global exception handler
}

void main()
// -----------------------------------------------------------------------------
// Func: Initialize IRQs and sleep
// -----------------------------------------------------------------------------
{
    // Enable IRQs
    __builtin_wrctl(0, 5); // Write 101 to status reg to enable all excps & IRQs
    __builtin_wrctl(3, 1); // Write 1  to ienable reg to enable Timer IRQ

    timCtrlPtr |= 0x1;     // Set ITO to detect interval timer interrupt
    
    while (FOREVER) {}     // Sleep
}
// -----------------------------------------------------------------------------