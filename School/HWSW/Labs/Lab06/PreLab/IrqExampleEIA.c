// ========================================================================= BOF
// Prog: IrqExampleV3.c
// Func: This file contains a simple program to perform a trivial function
//       in response to a pushbutton interrupt. It contains:
//       - func main      : Init. relevant port IRQs & go to sleep (that's all)
//       - func IrqParser : Identifies the IRQ source & calls the correct ISR
//       - func IsrButtons: ISR to service a pushbutton IRQ for this App.
//
// Revs: 2014.11.18 RMK: Cosmetic cleanups
//       2017.03.22 RMK: More Cosmetic Cleanups
//       2017.04.04 SWO: Added includes, prototypes, & Altera data types
//                       Updated pointers & properly configured interrupts
//       2017.08.23 RMK: Clarified previously cryptic func names & comments
//       2018.02.22 RMK: Fixed a missing "volatile". When did that happen?
//       2018.10.18 RMK: Cleaned up some ambiguities. Set version = V3.c
// -----------------------------------------------------------------------------
#include <alt_types.h>
#include "GenericExcepHndlr.c"     // Needed by Nios C compiler - DO NOT EDIT!

#define SWITCHES_BASE_ADDRESS     0x00011020    // Value TBD by QSys
#define RED_LED_BASE_ADDRESS      0x00011010    // Value TBD by QSys
#define BUTTONS_BASE_ADDRESS      0x00011000    // Value TBD by QSys

void ResetHandler (void);  // Function prototypes to make sure they compile
void ExcepHandler (void);
void IsrButtons   (void); 

// -----------------------------------------------------------------------------
// IRQ Source Parser for all external interrupts. 
// Func: Examine reg ctrl4 (ipending) to determine which device caused the IRQ,
//       then call the correct ISR to service that device. 
// Note: 1. In this example application, there is ONLY the Pushbutton ISR, 
//          but others can easily be edited into this function for other apps.
//       2. This example ASSUMES pushbutton IRQ is mapped to bit 1 of ctrl regs
//          ctl3 & ctl4. The ACTUAL mapping will be done by QSYS.
// -----------------------------------------------------------------------------
void IrqParser(void)             // called by global exception handler
{
    asm volatile
    (
        // Get the ipending control reg & test the butttons IRQ bit
        "ldw    r8, 0(ctrl4)    \n\t"
        "andi   r8, r8, 0x2     \n\t"
        // If the buttons IRQ bit is not set,
        // Then return (Since only 1 IRQ is enabled, this would be a bug)
        "beq    r8, r0, RETURN  \n\t"
        // Else do all the following to service the buttons IRQ:
        // Create a stack frame &
        // Push your return address and any registers you have altered.
        "subi r27, r27, 4       \n\t"
        "stw  r31, 0(r27)       \n\t"

        // Call the pushbutton ISR.
        "br   PBISR             \n\t"

        // Pop any registers you have altered & your return address.
        // Delete the stack frame.
        "ldw  r31, 0(r27)       \n\t"
        "addi r27, r27, 4       \n\t"

        // Return
        "RETURN:                \n\t"

        // Don't forget the output, input, and/or clobber lists as needed
        :
        :
        : "r8", "r27", "r31", "memory"
    );
}

// -----------------------------------------------------------------------------
// Application dependent Interrupt Service Routine (ISR) for Pushbutton IRQ.
// Func: In this trivial example, all it does is set each LED = value of its   
//       corresponding toggle switch at the instant Pushbutton 2 is pushed.  
// -----------------------------------------------------------------------------
void IsrButtons(void)           // called by IRQ Source Parser (IrqParser)
{
  volatile alt_u32 * redLeds  = (alt_u32 *) RED_LED_BASE_ADDRESS;
  volatile alt_u32 * buttons  = (alt_u32 *) BUTTONS_BASE_ADDRESS;
  volatile alt_u32 * switches = (alt_u32 *) SWITCHES_BASE_ADDRESS;

  *(redLeds) = *(switches);     // Make LEDs light up to match slide switches
  *(buttons + 3) = 0;           // Clear IRQ signal by writing to the
                                // edgecapture reg of the pushbutton PIO
  return;
}

// -----------------------------------------------------------------------------
// Main Program
// func: In this trivial example, all it does is initially enable the needed 
//       IRQs and sleep forever, letting the ISR do all the work.       
// -----------------------------------------------------------------------------
void main()
{
   volatile alt_u32 * buttonPtr = (alt_u32 *) BUTTONS_BASE_ADDRESS;
   __builtin_wrctl(0, 5);   // Write 5 to status reg to enable all exceps & IRQs
   __builtin_wrctl(3, 2);   // Write 2 to ienable reg to enable buttons IRQ

   *(buttonPtr + 2) = 0x4;  // Enable button 2 "KEY2" to cause an IRQ
 
   while(1){};              // Empty. In this pgm, ISR does all the work
}
// ========================================================================= EOF