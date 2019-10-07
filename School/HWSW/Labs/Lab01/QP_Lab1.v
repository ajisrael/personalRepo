// -----------------------------------------------------------------------------
// RMK 2018.01.31 - Bug fix in module SegDriver output declaration; 
//     moved "reg" to appear before vector limit specification.
//
// INSTRUCTOR'S NOTES:
// 1. The "reg" input & output in declarations may or may not be needed,
//    depending on which Verilog constructs you use. Delete, if appropriate.
//
// 2. The comments describe the functions of the INITIAL modules for this lab.
//    When you alter the functions for later sections of the lab, be sure to
//    edit the comments to describe your NEW functionality.

module QP_Lab1 (CLOCK_50, HEX0, HEX1, HEX2, HEX3, HEX4, HEX5, HEX6, HEX7
// -----------------------------------------------------------------------------
// TOP LEVEL MODULE: Coverts a 32-bit unsigned integer into driver signals for 
// the eight 7-Segment displays on the DE2-115 Board. This module instantiates:
// - one synchronous rollover up-counter and 
// - eight 7-segment display drivers (translators). 
// -----------------------------------------------------------------------------
   input           CLOCK_50;                // Sys. clock input to the FPGA.
   output   [6:0]  HEX0, HEX1, HEX2, HEX3,  // Standard DE2 pin names for the
                   HEX4, HEX5, HEX6, HEX7;  // 7-bit outputs to all 8 displays.
   wire     [31:0] numVal32;                // Full 32-bit counter value.

	// Instantiate counter
   Count Counter(CLOCK_50, numVal32);
	
	// Map counter values to Hex displays
	SegDriver Seg7(numVal[ 3:0 ], HEX0);
	SegDriver Seg7(numVal[ 7:4 ], HEX1);
	SegDriver Seg7(numVal[11:8 ], HEX2);
	SegDriver Seg7(numVal[15:12], HEX3);
	SegDriver Seg7(numVal[19:16], HEX4);
	SegDriver Seg7(numVal[23:20], HEX5);
	SegDriver Seg7(numVal[27:24], HEX6);
	SegDriver Seg7(numVal[31:28], HEX7);
	
endmodule

module Count (clock, ctr32);
// -----------------------------------------------------------------------------
// SUB-MODULE: Synchronous 32-bit rollover up counter
// -----------------------------------------------------------------------------
   input              clock;      // clock to drive the 23-bit counter.
   output reg [31:0]  ctr32;      // 32-bit counter value to output.

   initial ctr32 = 0;

	// Increase the counter by 1 every falling edge of the clock.
	always@(negedge clock)
	begin
		ctr32 = ctr32 + 1;
	end
	
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
			4'b1000 : outSig = 7'b1111111; // 8        4 |     | 2
			4'b1001 : outSig = 7'b0011000; // 9	          -----
			4'b1010 : outSig = 7'b0001000; // A             3
			4'b1011 : outSig = 7'b0000011; // b
			4'b1100 : outSig = 7'b1000110; // C
			4'b1101 : outSig = 7'b0100001; // d
			4'b1110 : outSig = 7'b0000110; // E
			4'b1111 : outSig = 7'b0001110; // F
			default : outSig = 7'b1111111; // off
	end

endmodule
