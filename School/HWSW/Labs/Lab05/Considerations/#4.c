// ============================================================================
#include <alt_types.h>

void SignBinToDec (alt_u8* strPtr, alt_32 num)
// ----------------------------------------------------------------------------
// Func: Checks the sign bit of a signed 32-bit integer and prepends a string
//       with the propper sign, and converts the decimal value into ASCII.
// Meth: Initailly assumes that the sign is positive and checks the most
//       significant bit. If it is a 1 then it changes the sign to negative, 
//       and does 2s compliment arithmetic to get the number's magnitude. It
//       then converts the number to an ASCII string and prepends the string
//       with the propper sign.
// Asum: It is assumed that the string is large enough to hold the number and a
//       sign at the beginning.
// Args: strPtr = Pointer to allocated string in memory.
//       num    = Signed 32-bit number to be converted to ASCII.
// Vars: sign   = ASCII value of sign of num.
// ----------------------------------------------------------------------------
{
	alt_u8 sign = '+';			// ASCII value of sign of num
	
    if (num[31] == 1)           // If num is negative
	{
		num ^= 0xFFFFFFFF;		// Flip all the bits
		num += 1;               // And add 1
		sign = '-';				// Change the sign
	}
	
	binToDec(strPtr, num);		// Convert to ASCII
	strPtr[0] = sign;			// Add sign to beginning of string
}
// ============================================================================
