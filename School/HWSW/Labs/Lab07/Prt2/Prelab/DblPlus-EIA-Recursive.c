//------------------------------------------------------------------------------
// Orig: 11.05.2019 - Alex Israels
// Prog: DblPlus_EIA_Recursive.c
// Func: Recursive EIA implementation of the DblPlus algorithm.
// Meth: Recursive version of DblPlus to be inserted into main immediately after 
//       starting the timer. Both defines and calls DblPlus.
// Vars: spFinal = Final value of the stack pointer
//       result  = Value to be returned to main
//       n       = Computes from 0 to n of the DblPlus Algorithm.
//------------------------------------------------------------------------------

asm volatile
(
    "call   DblPlus             \n\t"   // call DblPlus by its label
    "br     EndASM              \n\t"   // BR to asm exit after DblPlus returns

    // ----------------------------------------------------------- Start DblPlus
    "DblPlus:                         \n\t"   // label for address of function
    "   subi sp, sp, 4                \n\t"   // Decrement the stack pointer
    "   stw  ra, 0(sp)                \n\t"   // Push ra onto the stack
    "   or   %[RspFinal], sp, r0      \n\t"   // Read the stack pointer
    "   bne  %[Rn], r0, Recur         \n\t"   // Continue recursion if not base
    "   addi %[Result], r0, 1         \n\t"   // Set base case
    "   ret                           \n\t"   // Return

    "   Recur:                        \n\t"   // Recursive step in DblPlus
    "       subi %[Rn], %[Rn], 1      \n\t"   // Decrement N
    "       call DblPlus              \n\t"   // Start next recursive step
    "       addi %[Rn], %[Rn], 1      \n\t"   // Get value of N for step
    // Calculate result at current recursive step
    "       muli %[Result], %[Result], 2     \n\t"
    "       add  %[Result], %[Result], %[Rn] \n\t"
    "       ldw  ra, 0(sp)            \n\t"   // Pop ra off of stack
    "       addi sp, sp, 1            \n\t"   // Restore stack
    "       ret                       \n\t"   // return from function DblPlus
    // ------------------------------------------------------------- End DblPlus

    "EndASM:                    \n\t"   // label for end of asm statement

    // Output, Input, & Clobber lists here.
    :[RspFinal] "=r" (spFinal), [Result] "=r" (result), [Rn] "+r" (n)
    :
    :"memory"
);