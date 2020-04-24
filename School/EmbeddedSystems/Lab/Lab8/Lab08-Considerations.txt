// ==========================================================================BOF
// Name:  Lab08 - Considerations
// Orig:  2020.04.22 - Alex Israels
// Func:  Answer the questions to the considerations section of Lab 8.
// Ques:  1. Change packet to be in Fixed Length mode with a packet length of
//           6 Bytes:
//           a) Two TI_CC_SPIWriteReg calls to add:
                TI_CC_SPIWriteReg(TI_CCxxx0_PKTCTRL0, 0x03); // Fixed Len. Mode
                TI_CC_SPIWriteReg(TI_CCxxx0_PKTLEN,   0x06); // Length = 6
//           b) Change pktData:
static uint8_t pktData[6] = {0x41, 0x42, 0x43, 0x44, 0x45, 0x46}; // ASCII Chars
// Contents:   pktData[6] = {data, data, data, data, data, data}
//------------------------------------------------------------------------------

typedef union
{ uint8_t  u8[4];
  uint32_t u32;
} Uint32Payload_t;

void printDec(uint32_t binVal, char * decPtr)
//------------------------------------------------------------------------------
// Func: Convert a uint32_t number into its decimal representation.
//       Convert each decimal digit to an ASCII Character, and 
//       store the characters in a string of bytes.
// Args: binVal = the unsigned 32 bit binary value to be converted.
//       decPtr = pointer to the start of the ASCII character
//                sting representing the decimal value of binVal.
// Retn: void
//------------------------------------------------------------------------------
{ 
    uint32_t currVal = binVal;  // Variable to manipulate w/o altering orig.
    uint8_t i = 9;              // Maximum length of a 32 bit number in decimal
    
    while (currVal > 0)         // While there is a digit to convert
    {
        decPtr[i] = (currVal % 10) + 48;  // Get and convert the digit to ASCII
        i--;                              // Decrement pointer
        currVal /= 10;                    // Remove digit from number
    }
}
// ==========================================================================EOF