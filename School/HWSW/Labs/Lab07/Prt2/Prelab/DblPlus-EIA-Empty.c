//------------------------------------------------------------------------------
// Orig: 11.05.2019 - Alex Israels
// Prog: DblPlus_EIA_Empty.c
// Func: Tests the EIA implementation of DblPlus to make sure there are no
//       syntaxical errors.
// Meth: Empty version of DblPlus to be inserted into main immediately after 
//       starting the timer. Both defines and calls DblPlus.
// Vars: spFinal = Final value of the stack pointer
//------------------------------------------------------------------------------

asm volatile
(
    "call   DblPlus               \n\t"   // call DblPlus by its label
    "br     EndASM                \n\t"   // BR to asm exit after DblPlus returns

    // ----------------------------------------------------------- Start DblPlus
    "DblPlus:                     \n\t"   // label for address of function
    "   or   %[RspFinal], sp, r0  \n\t"   // Read the stack pointer
    "   ret                       \n\t"   // return from function DblPlus
    // ------------------------------------------------------------- End DblPlus

    "EndASM:                      \n\t"   // label for end of asm statement

    // Output, Input, & Clobber lists here.
    : [RspFinal] "=r" (spFinal),
    : 
    : "memory"
);