#include <alt_types.h>

void main()
{
	altu_16 word = 0;     // 16 bit half word from input
	altu_16 revWord = 0;  // 16 bit half to write to output.
	volatile altu_8 i;    // byte offset of input
	volatile altu_8 j;    // index counter for word and revWord

	volatile alt_32 * inputAddr = 0xFACE5000;   // address of input port
	volatile alt_32 * outputAddr = 0xFEED2000;	// address of output port

	// repeat
	do
	{
		// read 16 bit halfword from input port
		word = *(inputAddr + i);
		// reverse its byte-endian ordering
		for (j = 0; j < 16; j++)
		{
			revWord[j] = word[15-j]; 	
		}
		// write the new value to the output port
		*(outputAddr + i) = revWord
		i++;
	} 
	while (word != 0) // until the input word = 0
}