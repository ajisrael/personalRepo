//------------------------------------------------------------------------------
// Orig: 2019.10.14 - Alex Israels
// Prog: BinToDec.c
// Func: Using a pointer to the first element of an ASCII Character string 
//       buffer in memory, and a 32-bit unsigned integer. Converts the 32-bit
//       unsigned integer into a decimal number of the same numerical value.
// Meth: As each decimal digit is calculated the digit is stored as an alt_u8
//       variable, such that the digit occupies the least significant nybble of
//       the whole byte. The decimal digit is then encoded into its ASCII
//       character, suitable for printing onscreen. Finally, that digit is
//       stored in the correct entry in the buffer whose pointer was passed in
//       the arguments. The character string is then null terminated and all
//       unused "leading" characters in the string are padded with the blank
//       "space" character.
// Args: strPtr = Pointer to first element in ASCII Character string buffer.
//       num    = Unsigned integer to be converted to a decimal number.
//------------------------------------------------------------------------------
#include <alt_types.h>

#define ASCII_OFFSET 48 // Decimal value offset btw a Dec# and its ASCII value.

void BinToDec (alt_u32 * strPtr, alt_u32 num)
{
    alt_u8 digit = 0;
    while (num != 0)                  // While there are still digits to read
    {
        digit = num % 10;             // Calculate & store next decimal digit
        num = num / 10;               // Remove least significant digit
        digit = digit + ASCII_OFFSET; // Encode digit to ASCII
        // Store digit to correct pos in buffer

        // Null-Terminate character string

        // Pad out leading characters with ' '
    }
}
//------------------------------------------------------------------------------