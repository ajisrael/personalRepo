alt_u32 DblPlus(alt_u32 n)
{
// -----------------------------------------------------------------------------
// Func: Recursively calculate the following recursive function.
//       DblPlus(n) = n + 2 * DblPlus(n) & DblPlus(0) = 1
// Args: n = value of n to be doubled
// -----------------------------------------------------------------------------
	asm("or %[RspFinal], sp, r0 \n\t"   // Read the stack pointer
		:[RspFinal] "=r" (spFinal)
		:
		:"memory"
		);

	alt_u32 result = 1;
	if(n != 0)
	{
		result = n + 2 * DblPlus(n - 1);
	}
	
	return result;
}