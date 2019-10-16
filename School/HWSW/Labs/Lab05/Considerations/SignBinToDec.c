void BinToDec (alt_u8 * strPtr, alt_32 num)
//------------------------------------------------------------------------------
// Func: Using a pointer to the first element of an ASCII Character string 
//       buffer in memory, and a 32-bit signed integer. Converts the 32-bit
//       signed integer into a decimal number of the same numerical value.
// Meth: As each decimal digit is calculated the digit is stored as an alt_u8
//       variable, such that the digit occupies the least significant nybble of
//       the whole byte. The decimal digit is then encoded into its ASCII
//       character, suitable for printing onscreen. Finally, that digit is
//       stored in the correct entry in the buffer whose pointer was passed in
//       the arguments. The character string is then null terminated and all
//       unused "leading" characters in the string are padded with the blank
//       "space" character.
// Args: strPtr = Pointer to first element in ASCII Character string buffer.
//       num    = Signed integer to be converted to an ASCII Character.
// Vars: digit  = Current digit of num being converted.
//       pos    = Current position in strPtr. Set to 10 b/c strPtr will always
//                point to a 12 element null terminated array.
//       i      = Index variable for padding space characters.
//------------------------------------------------------------------------------
{
    alt_u8 digit = 0;                 // Current digit of num being converted
    alt_u8 pos   = 10;                // Current position in strPtr

    while (num != 0)                  // While there are still digits to read
    {
        digit = num % 10;             // Calculate & store next decimal digit
        num = num / 10;               // Remove least significant digit
        digit = digit + ASCII_OFFSET; // Encode digit to ASCII
        strPtr[pos] = digit;          // Store digit to correct pos in buffer
        pos--;
    }

    for (int i = 0; i <= pos, i++)    // Loop over remaining characters
    {
        strPtr[i] = ' ';              // Pad leading chars with ' '
    }

    strPtr[12] = 0x00;                // Null-Terminate character string
}
//------------------------------------------------------------------------------