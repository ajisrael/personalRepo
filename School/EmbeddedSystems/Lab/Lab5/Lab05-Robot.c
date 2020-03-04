// ========================================================================= BOF
// Orig: 2020.03.04 - Alex Israels
// Func: Send a pre-defined sequence of commands to Robot via UART
//       Triggered by Timer A0 TAR Rollover IRQ (TAIV)
// Meth: TimerA is configured in UP mode with TACCR0 set to a value to make the
//       period 2 seconds. TAIFG is enabled to allow for an ISR to send the
//       command to the robot every 2 seconds.
// Defn: PD_2SEC    = The number of TACLK cycles to achieve a 2 second period.
//       FWD_SP0_50 = Command to drive both motors forward at half speed.
//       STOP_STAY  = Command to stop both motors (not global all-stop cmd).
//       SPN_SP0_25 = Command to spin robot at quarter speed.
// Rev:  None (yet)
//-----------------------------------------------------------------------------

#include "msp430x22x4.h"
#include "stdint.h"

#define PD_2SEC    0x5DC0  // 2 second period @ 12 KHz (TA PD w/ ACLK source)
#define FWD_SP0_50 0x60E0  // M1 FWD @ .50 spd [96] : M2 FWD @ .50 spd [224]
#define STOP_STAY  0x40C0  // M1 Stop & Stay   [64] : M2 Stop & Stay   [192]
#define SPN_SP0_25 0x30D0  // M1 RVS @ .25 spd [48] : M2 FWD @ .25 spd [208]

union                              // Union for 8 Sabertooth command bytes
{
    uint16_t u16[4];
    uint8_t  u8[8];
}   cmdSet;

cmdSet.u16[0] = FWD_SP0_50;        // 1st cmd go forward @ half speed for 2 sec
cmdSet.u16[1] =  STOP_STAY;        // 2nd cmd stop and wait for 2 sec
cmdSet.u16[2] = SPN_SP0_25;        // 3rd cmd spin @ quarter speed for 2 sec
cmdSet.u16[3] =  STOP_STAY;        // 4th cmd stop and stay (forever)

#pragma vector = TIMERA1_VECTOR
__interrupt void IsrRoboCmds(void)
//------------------------------------------------------------------------------
// Trig:  TIMER A0 TAR Rollover IRQ (TAIV)
// Func:  Create and output a pair of commands to Sabertooth robot contoller;
//        one cmd. to left-side motors & one cmd. to right-side motors.
// Meth:  Using a union of predefined commands, cmd and i are used to map to 
//        the correct command in the cmdSet union. The command is sent after
//        waiting for the TX register to be clear on UCA.
// Args:  None
// Vars:  entry = Index for which command pairs to send.
//        i     = Index to the command byte to send.
// Retn:  None
//------------------------------------------------------------------------------
{
    static uint8_t cmd = 0;       // Init. indes for which command pair to send
    uint8_t i = 0;                // Init. index of command byte to send

    switch (__even_in_range(TAIV, 10)) // I.D. source of the TA IRQ
    {
    case TAIV_TAIFG: // Handle TAR rollover IRQ

        while (i < 2)                       // Send only 2 Bytes at a time
        {
            while (!(IFG2 & UCA0TXIFG)) {}; // Wait for TX buff to be ready
            UCA0TXBUF = cmdSet.u8[cmd+i];   // Send command
            i++;                            // Increment to next command
        }

        cmd = cmd + 2;                      // Prepare for next pair of cmds

        if (cmd > 7)                        // Check if all cmds have been sent
        {
            TACTL &= ~(TAIE);               // Disable further interrupts
        }
        break;
        
    case TAIV_TACCR1: // ignore chnl 1 IRQ
    case TAIV_TACCR2: // ignore chnl 2 IRQ
    default:          // ignore everything else
    }
}

void main(void)
//------------------------------------------------------------------------------
// Func:  Configure UART  functions & Ports,
//        Configure Timer functions & params,
//        Go to sleep & let Timer ISR do all of the work.
// Args:  None
// Retn:  None
//------------------------------------------------------------------------------
{
    WDTCTL = WDTPW + WDTHOLD; // Stop watch dog
    P3SEL = 0x30;             // P3.4 & 5 = USCI_A0 TXD & RXD

    //......................................Config UART Clock & Baud Rate
    DCOCTL = CALDCO_1MHZ;  // DCO = 1 MHz
    BCSCTL1 = CALBC1_1MHZ; // DCO = 1 MHz
    UCA0CTL1 |= UCSSEL_2;  // UART uses SMCLK @ 1 MHz

    UCA0MCTL = UCBRS0; // Map 1MHz -> 9600 (Tbl 15-4)
    UCA0BR0 = 104;     // Map 1MHz -> 9600 (Tbl 15-4)
    UCA0BR1 = 0;       // Map 1MHz -> 9600 (Tbl 15-4)

    //......................................Init UART Output = All Stop
    UCA0CTL1 &= ~UCSWRST;                   // Enable USCI (unreset)
    while (!(IFG2 & UCA0TXIFG)){};          // Confirm that Tx Buff is empty
    UCA0TXBUF = 0x00; // Init: send robot a stop command

    //......................................Config TA to approx 2 sec rollover
    BCSCTL3 |= LFXT1S_2; // ACLK <-- VLO (approx 12 KHz)

    TACTL |= TASSEL_1 | MC_1 | TAIE;        // SEL ACLK | Up mode | Enable IRQ
    TACCR0 = PD_2SEC - 1;                   // Set period to 2 sec
    while (!(TACTL & TAIFG)){};             // Wait for 2 second delay

    //.......................................Enter LPM0 with IRQs enabled
    __bis_SR_register(LPM0_bits | GIE);
}
//========================================================================== EOF