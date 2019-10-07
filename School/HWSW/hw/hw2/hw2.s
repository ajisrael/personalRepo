# ========================================================================== BOF
# Orig: 2019.09.28 - Alex Israels
# Func: Takes in half word from input port and reveses its endian order and 
#		writes it to the output port.
# r0  = Holds the value 0 for convenient conditional comparison
# r8  = Address of the input port
# r9  = Address of the output port
# r10 = Value at the input port
# r11 = Address of buffer for reversing endian order
# r12 = Forloop counter
# r13 = byte offset for reversing endian order
# r14 = complement offset of r13
# r15 = Byte from input port to be put at opposite location of buffer
# ------------------------------------------------------------------------------

.include "nios_macros.s"  	# defines pseudo-instructions

.text                     	# Load code into the .text partition of memory
.global _start            	# Make _start available outside program
_start:                   	# Default label for 1st address in program

	movia r8, INPUT 		# R8 = ADDR of INPUT
	ldw r8, 0(r8)			# R8 = VALUE of INPUT = ADDR of input port

	movia r9, OUTPUT 		# R9 = ADDR of OUTPUT
	ldw r9, 0(r9)			# R9 = VALUE of OUTPUT = ADDR of output port

	ldhuio r10, 0(r8)		# R10 = VALUE at input port

	movia r11, BUF			# R11 = 16 byte block for reversing endian order

MAIN:
	beq r0, r10, END		# IF value at input port == 0, go to DONE
	br REVERSE				# ELSE reverse the endian order

REVERSE:
	movui r12, MAX_CNT		# R12 = count of for loop = 16
	subi r13, r12, 1		# R13 = byte offset for reversing endian order
LOOP:
	sub r14, OFF_CNT, r13 	# R14 = complement offset of R13 = 15 - R13
	ldbuio r15, r14(r8) 	# R15 = byte from input at R14 offset
	stbio r13(r11), r15 	# Store R15 in complement location of buffer
	subi r12, r12, 1		# Decrement for loop counter
	subi r13, r13, 1		# Decrement offset counter
	bne r12, r0, LOOP		# IF R12 != 0, go to top of for loop
	br OUT 					# ELSE go to store to output port

OUT:
	sthio r9, r11 			# Store reversed value to output port
	ldw r8, 1(r8)			# Move to next location of input ADDR
	ldhuio r10, 0(r8)		# Load next half word
	br MAIN					# Go back to main

END:
	br END 					# Give time to view results

# ------------------------------------------------------------------------------

.data                       # Load data into the .data partition of memory

INPUT:						# Label of INPUT = ADDR
.word 0xFACE5000			# ADDR of input port

OUTPUT:						# Label of OUTPUT = ADDR
.word 0xFEED2000			# ADDR of output port

BUF:						# Label BUF = ADDRESS for reversed endian buffer
.skip 16					# Reserve 16 bytes of memory w/o initializing it

MAX_CNT:					# Label MAX_CNT = imm 16
.equ 16						# Sets value of MAX_CNT to 16

OFF_CNT:					# Label OFF_CNT = imm 15
.equ 15						# Sets value of MAX_CNT to 15

.end

# ========================================================================== EOF
