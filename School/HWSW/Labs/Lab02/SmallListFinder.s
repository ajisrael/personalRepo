# ========================================================================== BOF
# Orig: 2015.09.14 - Kit Cischke & Roger Kieckhafer 
# Rev : 2019.09.21 - Alex Israels
# Func: Increments through a list of values to find the smallest one and
#       stores it in memory.
# r0  = Holds the value 0 for convenient conditional comparison
# r8  = Number of items in the list
# r9  = Address of current item in the list to be compared
# r10 = Value of current smallest item in the list
# r13 = Smallest value in list at end of program
# r14 = Boolean for comparison between r10 and r15
# r15 = Value of current item in the list to be compared
# ------------------------------------------------------------------------------

.include "nios_macros.s"  # defines pseudo-instructions

.text                     # Load code into the .text partition of memory
.global _start            # Make _start available outside program
_start:                   # Default label for 1st address in program

  movia r8, N             # R8 = ADDR. of N (movia loads a LABEL, not a VALUE)
  ldw r8, 0(r8)           # R8 = VALUE of N = num of items in list
  
  movia r9, ENTRIES       # R9  = ADDR. of 1st item in list (list pointer)
  ldw r10, 0(r9)          # R10 = VALUE of 1st (and so far) smallest value

LOOP: 
  subi r8, r8, 1          # Decrement counter of items left to process
  beq r8, r0, DONE        # Break if counter = 0 (=> no items left)
  addi r9, r9, 4          # Increment list pointer by one word (4 bytes)
  ldw r15, 0(r9)          # Get next value in list
  cmpgtu r14, r10, r15	  # Compare smallest value to the next value
  beq r0, r14, LOOP       # IF   smallest value <= next value, go back to top
  mov r10, r15            # ELSE Save new smallest value to r10
  br LOOP                 # Loop to top & get next value from list
 
DONE: 
  movia r13, SMALL        # Get ADDRESS reserved for the smallest value
  stw r10, 0(r13)         # Store smallest value to memory
 
END:
  br END                  # Infinite Loop lets user view the results

# ------------------------------------------------------------------------------

.data                       # Load data into the .data partition of memory

ENTRIES:                    # Label ENTRIES = ADDRESS of the data list
.word 4,5,3,6,9,8,2         # Initialize the VALUES of the entries of the list

N:                          # Label of N = ADDRESS of number of entries in list
.word 7                     # Value of N = Number of entries in list

SMALL:                      # Label SMALL = ADDRESS for the smallest value
.skip 4                     # Reserve (but don't initialize) 4 Bytes of memory
                            # into which to store the final smallest value.
.end

# ========================================================================== EOF