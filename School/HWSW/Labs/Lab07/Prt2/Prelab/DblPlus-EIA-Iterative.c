//------------------------------------------------------------------------------
// Orig: 11.05.2019 - Alex Israels
// Prog: DblPlus_EIA_Iterative.c
// Func: Iterative EIA implementation of the DblPlus algorithm.
// Meth: Iterative version of DblPlus to be inserted into main immediately after 
//       starting the timer. Both defines and calls DblPlus.
// Vars: spFinal = Final value of the stack pointer
//       result  = Value to be returned to main
//       n       = Computes from 0 to n of the DblPlus Algorithm.
//------------------------------------------------------------------------------

asm volatile
(
    "call   DblPlus               \n\t"   // call DblPlus by its label
    "br     EndASM                \n\t"   // BR to asm exit after DblPlus returns

    // ----------------------------------------------------------- Start DblPlus
    "DblPlus:                     \n\t"   // label for address of function
    "   or   %[RspFinal], sp, r0  \n\t"   // Read the stack pointer
    "   addi %[Result], r0, 1     \n\t"   // Initialize result
    "   addi r8, r0, 1            \n\t"   // Initialize loop counter
    
    "   Loop:                     \n\t"   // Start iterative Loop
    "       subi %[Rn], $[Rn], 1  \n\t"   // Decrement n
    "       addi r8, r8, 1        \n\t"   // Increase the counter
    // Calculate new result
    "       muli %[Result], %[Result], 2  \n\t"
    "       add  %[Result], %[Result], r8 \n\t"
    "       beq  %[Rn], r0, Retn  \n\t"   // Return if done
    "       br   Loop             \n\t"   // Else continue loop

    "   Retn:                     \n\t"   // Return call of Dbl Plus
    "       ret                   \n\t"   // return from function DblPlus
    // ------------------------------------------------------------- End DblPlus

    "EndASM:                      \n\t"   // label for end of asm statement

    // Output, Input, & Clobber lists here.
    : [RspFinal] "=r" (spFinal), [Result] "=r" (result), [Rn] "+r" (n)
    : 
    : "memory"
);