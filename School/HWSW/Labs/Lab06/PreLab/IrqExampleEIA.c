// ========================================================================= BOF
// Orig: Roger M. Kieckhafer
// Revs: 2019.10.22 - Alex Israels
// Prog: IrqExampleEIA.c
// Func: Implements an ISR resulting from an IRQ from the pushbuttons on the 
//       Nios II.
// Meth: The IRQ is parsed using EIA to verify the IRQ came from the pushbutton
//       and manages the stack accordingly.
// Defn: SWITCHES_BASE_ADDRESS = Base address of the switch PIO.
//       RED_LED_BASE_ADDRESS  = Base address of the RLED PIO.
//       BUTTONS_BASE_ADDRESS  = Base address of the buttion PIO.
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
        "ldw    r8, 0(ctrl4)    \n\t"   // Get ipending contents
        "andi   r8, r8, 0x2     \n\t"   // Check if buttons IRQ pending
        // If the buttons IRQ bit is not set,
        // Then return (Since only 1 IRQ is enabled, this would be a bug)
        "beq    r8, r0, RETURN  \n\t"   // Return
        // Else do all the following to service the buttons IRQ:
        // Create a stack frame &
        // Push your return address and any registers you have altered.
        "subi r27, r27, 8       \n\t"   // Make space for stack frame
        "stw  r31, 4(r27)       \n\t"   // Save return address
        "stw  r8,  0(r27)       \n\t"   // Save r8 (JIC)

        // Call the pushbutton ISR.
        "call IsrButtons        \n\t"   // call ISR

        // Pop any registers you have altered & your return address.
        // Delete the stack frame.
        "ldw  r31, 4(r27)       \n\t"   // Load return address
        "stw  r8,  0(r27)       \n\t"   // Load r8 (JIC)
        "addi r27, r27, 8       \n\t"   // Delete stack frame

        // Return to regularly scheduled program.
        "RETURN:                \n\t"   // Finish

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