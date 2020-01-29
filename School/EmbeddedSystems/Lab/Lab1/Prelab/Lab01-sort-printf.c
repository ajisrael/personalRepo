// ============================================================================
// Func: Sort a list of 16-bit signed integers into ascending order.
// Meth: Bubble Sort, executed in main.
//       Yes, we know Bubble Sort is dumb, but it makes an easy demo.
// Rev:  2015.01.29: C. Jiang changed lower limit on index i
//       fm "i = 0", to "i = 1" to correct list over-run. Tested Satis.
//       2018.06.08: Minor updates & corrections.
//       2020.01.29: A. Israels added print statement to terminal.
//-----------------------------------------------------------------------------

#include "msp430x22x4.h" // Brobdingnagian chip-specific macros & defs
#include "stdint.h"      // MSP430 data type definitions
#include "stdio.h"       // allows comm. with host terminal
#define LIST_LEN 10      // Define length of list to be sorted

void main(void)
//-----------------------------------------------------------------------------
// Func: Define list length & constant list values, then sort the list.
// Args: None
// Retn: None
//-----------------------------------------------------------------------------
{
    WDTCTL = WDTPW | WDTHOLD; // Stop Watchdog Timer
    uint8_t i, j;             // loop indices
    int16_t swapBuf;          // one word temp. swap buffer
    int16_t list[LIST_LEN] =  // init constant vals in list
        {0x000C, 0x0C62, 0x0180, 0xBEEF, 0x00E0,
         0x0CCF, 0x0C35, 0x069E, 0x02F4, 0xDEED};

    for (i = 1; i < LIST_LEN; i++)
    {
        for (j = 0; j < LIST_LEN - i; j++)
        {
            if (list[j + 1] < list[j]) // if 2 elements are out of order
            {
                swapBuf = list[j];     // move larger  value to swap buf
                list[j] = list[j + 1]; // move smaller value down
                list[j + 1] = swapBuf; // move larger  value up
            }
        }
    }
    for (i = 0; i < LIST_LEN; i++) // print entire list to terminal (stdout)
    {
        printf(" %u\t %d\t %X\n ", list[i], list[i], list[i]);
    }
    list[0] = list[0]; // useless statement, but good place to put breakpoint
}
// ============================================================================