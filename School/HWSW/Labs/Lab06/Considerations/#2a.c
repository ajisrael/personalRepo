# define JTUA_ADDR    0x00081088	// Base address of JTAG UART
# define TIME_ADDR    0x00081020    // Base address of timer
# define BUTN_ADDR    0x00081060    // Base address of pushbutton PIO

// Initialize registers to JTAG UART: Registers are 32-bits, offset = 4 bytes
volatile alt_u32 * juCtrlPtr  = (alt_u32*) (JTUA_ADDR + 4); // Ptr to ctrl. reg.

// Initialize registers to Timer: Registers are 16-bits, offset = 2 bytes
volatile alt_u16 * timCtrlPtr = (alt_u16*) (TIME_ADDR + 4); // Ptr to ctrl. reg.

// Initialize registers to PushButton PIO: 32-bit reg., offset = 4 bytes
volatile alt_u32 * pbInMsPtr  = (alt_u32*) (BUTN_ADDR + 8); // Ptr to irq mask

void main ()
{
    alt_u32 tmpReg;                // 32-bit temporary variable
        
    // Globally enable exceptions and IRQs
    tmpReg = __builtin_rdctl(0) | 0x5;
    __builtin_wrctl(0, tmpReg);
    
    // Enable timer, JTAG, and pushbutton IRQs
    tmpReg = __builtin_rdctl(3) | 0x103;
    __builtin_wrctl(3, tmpReg);

    *ledPtr = 0x00000000;          // Initialize all LEDs to OFF
    *timCtrlPtr |= 0x07;           // Set timer to CONT and enable ITO
                                   // & set the START bit to start timer

    *juCtrlPtr  |= 0x2             // Enable read and write interrupts
    *pbInMsPtr  |= 0xF             // Enable interrupt on pushbuttons

    while (FOREVER) {}             // Wait for an IRQ to occur...
}