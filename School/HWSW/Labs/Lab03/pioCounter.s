# ========================================================================== BOF
# Orig: 2019.10.01 - Alex Israels
# Func: Increases a counter infinitely and "writes" it to the PIO address 
#       provided by Quartus. (Need to manually enter the PIO Address.)
# r0  = Holds the value 0 for convenient conditional comparison
# r8  = Address of PIO
# r9  = Counter value to be written to PIO
# ------------------------------------------------------------------------------

.include "nios_macros.s"  # defines pseudo-instructions

.text                     # Load code into the .text partition of memory
.global _start            # Make _start available outside program
_start:                   # Default label for 1st address in program

movia r8, PIO             # R8 = ADDR of PIO

addi r9, r0, 0            # R9 = counter value

LOOP:
   addi  r9, r9, 1        # Increment counter by 1
   stwio r8, r9           # Write value of counter to PIO
   br LOOP                # Loop infinitely

# ------------------------------------------------------------------------------

.data

PIO:                      # Label PIO = ADDRESS of PIO
.equ 0xdeadbeef

.end

# ========================================================================== EOF