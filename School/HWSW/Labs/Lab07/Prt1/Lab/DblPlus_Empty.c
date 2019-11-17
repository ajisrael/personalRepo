alt_u32 DblPlus(alt_u32 n)
{
// -----------------------------------------------------------------------------
// Func: Used to calulate fixed overhead delays in timing.
// Args: n = Unused
// -----------------------------------------------------------------------------
	asm("or %[RspFinal], sp, r0 \n\t"   // Read the stack pointer
		:[RspFinal] "=r" (spFinal)
		);

	return -1;
}