// ------------------------------------------------------------------------------
// Orig: 2019.10.01 - Alex Israels
// Func: Writes out 32-bit input value to 8 7-segment displays in hex.
// Args: numVal32 = 32-bit input value to display
//       HEX[0:7] = All 8 7-bit 7-segment displays
// ------------------------------------------------------------------------------

module SegDriver32 (numVal32, HEX0, HEX1, HEX2, HEX3, HEX4, HEX5, HEX6, HEX7);
// -----------------------------------------------------------------------------
// SUB-MODULE: Translates 32-bit unsigned input value into instantiated 7-bit 
// output signals to display one hexadecimal digit on 8 7-segment displays.
// -----------------------------------------------------------------------------
   input     [31:0]  inVal;                  // 32-bit input value to display
   output    [ 6:0]  HEX0, HEX1, HEX2, HEX3, // 7-bit 7-segment displays
                     HEX4, HEX5, HEX6, HEX7;

   SegDriver Seg0(numVal32[ 3:0 ], HEX0);
   SegDriver Seg1(numVal32[ 7:4 ], HEX1);
   SegDriver Seg2(numVal32[11:8 ], HEX2);
   SegDriver Seg3(numVal32[15:12], HEX3);
   SegDriver Seg4(numVal32[19:16], HEX4);
   SegDriver Seg5(numVal32[23:20], HEX5);
   SegDriver Seg6(numVal32[27:24], HEX6);
   SegDriver Seg7(numVal32[31:28], HEX7);

endmodule

module SegDriver (inVal, outSig);
// -----------------------------------------------------------------------------
// SUB-MODULE: Translates 4-bit unsigned input value into  7-bit output signals 
// to display one hexadecimal digit on one 7-segment display. Remember: the
// segment bits are labeled little endian, 6...0, not 0...6 
// -----------------------------------------------------------------------------
   input      [3:0] inVal;    // 4-bit Hex input value to display.
   output reg [6:0] outSig;   // 7-bit Segment to output to display.

   // When inVal changes the segment is updated.
	always@(inVal)
	begin
		case(inVal)
			4'b0000 : outSig = 7'b1000000; // 0
			4'b0001 : outSig = 7'b1111001; // 1    Layout:
			4'b0010 : outSig = 7'b0100100; // 2             0
			4'b0011 : outSig = 7'b0110000; // 3           -----
			4'b0100 : outSig = 7'b0011001; // 4          |     |
			4'b0101 : outSig = 7'b0010010; // 5        5 |     | 1
			4'b0110 : outSig = 7'b0000010; // 6           --6--
			4'b0111 : outSig = 7'b1111000; // 7          |     |
			4'b1000 : outSig = 7'b0000000; // 8        4 |     | 2
			4'b1001 : outSig = 7'b0011000; // 9	          -----
			4'b1010 : outSig = 7'b0001000; // A             3
			4'b1011 : outSig = 7'b0000011; // b
			4'b1100 : outSig = 7'b1000110; // C
			4'b1101 : outSig = 7'b0100001; // d
			4'b1110 : outSig = 7'b0000110; // E
			4'b1111 : outSig = 7'b0001110; // F
			default : outSig = 7'b1111111; // off
		endcase
	end

endmodule


