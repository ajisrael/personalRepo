// ================================================================================BOF
// Orig: 2020.06.02 - Alex Israels
// Func: Sends data via infrared when a trigger is pressed all using interrupts so the
//       main loop is left empty.
// Meth: Timer/Counter5 creates a 56KHz carrier frequency for the IR signal tied to
//       pin 46 (OC5A). Timer/Counter4 is used to send the encoded data when the
//       trigger button is pressed by changing the mode of pin 46 to allow the
//       carrier frequency signal through for a specific ammount of time for a 1 or 0.
// Defn: TIME_BASE     = # of 16MHz clk cycles for Timer4 to overflow after 600us
//       TIMER_MAX     = Maximum value of 16-bit timers
//       TIME_HEADER   = # of 16MHz clk cycles for Timer4 to overflow after 2400us
//       PACKET_LENGTH = Default length of packet in bytes
//       DELAY_10MS    = Delay value of 10 ms in us
// Cont: IRLEDMASK     = Mask for the IRLED pin in DDRL (Data Direction Reg L)
//       MSB_Byte      = Mask for most significant bit of a byte
//       triggerPin    = The pin attached to the trigger (must be a HW interrupt pin)
//       baseDelay     = Base delay of 600us for Timer4
//       headerDelay   = Header delay of 2400 us for Timer4
// Vars: triggerDelay  = Delay for trigger to prevent spamming
//       packetLength  = The current length of the packetLength
//       data          = Array of data to transmit
//       dataBit       = Index of which bit of current byte in data array to transmit
//       dataByte      = Index of which byte of data array to transmit
// Rev:  2020.06.05 - Alex Israels:
//                  - Integrated timer4 to control output of IRLED pin with 600 us
//                    time base and variable duty cycle to correctly transmit data.
//       2020.06.08 - Alex Israels:
//                  - Added capability to T4_OVF_ISR to be able to send multiple
//                    bytes of data.
//       2020.06.09 - Alex Israels:
//                  - Added comments and documentation.
//-----------------------------------------------------------------------------------

// Definitions
#define TIME_BASE      9600 // Number of 16MHz clk cycles for 600us timer (bit delay)
#define TIMER_MAX     65536 // Max value of 16-bit timer before overflow
#define TIME_HEADER   38400 // Number of 16MHz clk cycles for 2400us (header delay)
#define PACKET_LENGTH     4 // Default length of the packet being sent in bytes
#define DELAY_10MS   100000 // Delay value of 10 ms in us

// Byte masks
const byte IRLEDMASK = B00001000;      // Mask for the IRLED pin in DDRL
const byte MSB_Byte = B10000000;       // Mask for most significant bit of a byte

// Pin definitions
const byte triggerPin = 2;             // Pin attached to trigger

// Fixed delay variables
const int baseDelay = TIMER_MAX - TIME_BASE;     // Base delay of 600 us for timer4
const int headerDelay = TIMER_MAX - TIME_HEADER; // Header delay of 2400 us for timer4

// Flexible delay variables
int  triggerDelay = DELAY_10MS;                    // Delay for trigger (no spamming)

// Packet variables
byte packetLength = PACKET_LENGTH;                 // The current length of the packet
byte data[PACKET_LENGTH] = {'*', '*', '7', '*'};   // Data to send
byte dataBit = MSB_Byte;                           // Index of which data bit
byte dataByte = 0;                                 // Index of which data byte

void initFireTimer() 
// ------------------------------------------------------------------------------------
// Func: Initializes the registers for Timer 4 to alternate the mode of OC5A in the
//       TIMER4_OVF ISR. This will then be used to encode the data for the IR signal.
// Meth: Turns off the timer and initializes the counter. Also makes sure PL3 (pin 46)
//       is set to input mode to prevent misfire.
// Refs: TCCR4A = Timer/Counter4 Control Register A
//       TCCR4B = Timer/Counter4 Control Register B
//       DDRL   = Port L Data Direction Register
// ------------------------------------------------------------------------------------
{
  TCCR4A = 0;     // clear registers and turn timer off
  TCCR4B = 0;
  DDRL &= ~PL3;   // set pinmode of PL3 (pin 46) to input (no output on boot)
}

void init56KHzCarrierFreq() 
// ------------------------------------------------------------------------------------
// Func: Initializes the registers for Timer 5 to generate a 56KHz PWM signal with a
//       50% duty cycle. This will be the carrier frequency for the IR.
// Meth: Sets the timer to Fast PWM mode to set OC5A (pin 46) high on a match and 
//       reset when ICR5 is met. The values were calculated as 35 for ICR5 and 17 for 
//       OCR5A as shown in their respective comments.
// Refs: TCCR5A = Timer/Counter5 Control Register A
//       TCCR5B = Timer/Counter5 Control Register B
//       ICR5   = Input Compare Register 5
//       OCR5A  = Output Compare Register 5A
// ------------------------------------------------------------------------------------
{
  TCCR5A = 0; // Clear timer registers
  TCCR5B = 0; 
  TCCR5A |= (1<<WGM11) | (1<<COM5A1) | (1<<COM5B1); // Fast PWM, Set OC5A high on match
  TCCR5B |=  (1<<CS11) |  (1<<WGM12) |  (1<<WGM13); // prescaler 8,  Fast PWM IRC5 Top
  ICR5 = 35;  // 16MHz / 8 = 2MHz / 56KHz = 35.7 -> 36 - 1 (0 index) = 35
  OCR5A = 17; // 36 / 2 - 1 (0 index) = 17
}

void triggerISR()
// ------------------------------------------------------------------------------------
// Func: ISR for when the trigger is pulled to transmit the data packet.
// Meth: Currently, implements a temporary delay to debounce the trigger since it is a
//       pushbutton. Then sets TCNT4 to overflow after 2400 us (length of the header
//       field of a data packet). Followed by setting the IRLED pin to output mode
//       to start the 56KHz wave managed by timer 5. The overflow interrupt is enabled
//       for timer 4 and then the timer is started.
// Refs: TCNT4  = Timer/Counter4 counter register value
//       DDRL   = Port L Data Direction Register
//       TIMSK4 = Timer/Counter4 interrupt enable mask
//       TCCR4B = Timer/Counter4 Control Register B
// ------------------------------------------------------------------------------------
{ 
  delayMicroseconds(triggerDelay); // Debouce trigger TODO: Remove w/ hardware upgrade

  TCNT4 = headerDelay;    // Delay for header of data packet
  DDRL |= IRLEDMASK;      // Set PL3 to output mode
  TIMSK4 |= (1<<TOIE4);   // enable interrupt
  TCCR4B |= (1<<CS40);    // prescaler = 1 and start timer
}

ISR(TIMER4_OVF_vect)
// ------------------------------------------------------------------------------------
// Func: ISR for timer 4 overflow. Sets the delay according to next bit sent to 
//       transmit the data packet according to the packet structure.
// Meth: Checks for if the packet has completed being sent and resets index variables
//       if complete. When not complete it sets the counter for timer 4 to value for a
//       base delay of 600 us. Then checks if the last cycle was low (delimitter for a
//       bit) and adds additional delay when transmitting a 1.
// Refs: TCNT4  = Timer/Counter4 counter register value
//       DDRL   = Port L Data Direction Register
//       TIMSK4 = Timer/Counter4 interrupt enable mask
//       TCCR4B = Timer/Counter4 Control Register B
// ------------------------------------------------------------------------------------
{
  if (dataBit == 0)                 // After Xmit byte complete
  {
    dataBit = MSB_Byte;             // reset bit index
    packetLength--;                 // Mark 1 byte sent
    dataByte++;                     // increment to next byte

    if (packetLength == 0)          // Check for end of packet
    {
      DDRL   |=  IRLEDMASK;         // Set IRLED high for toggle off
      TIMSK4 &= ~(1<<TOIE4);        // disable interrupt
      TCCR4B &= ~(1<<CS40);         // turn off timer
      packetLength = PACKET_LENGTH; // reset packet length
      dataByte = 0;                 // reset byte index
    }
  }
  
  TCNT4 = baseDelay;                // set timer for 600us delay
  
  if (!(DDRL & IRLEDMASK))          // if last cycle was low
  {
    if (data[dataByte] & dataBit)   // and xmitting a 1
    {
      TCNT4 -= TIME_BASE;           // increase delay to 1200us
    }
    dataBit /= 2;                   // increment to next bit
  }
  
  DDRL ^= IRLEDMASK;                // toggle IRLED
}

void setup()
// ------------------------------------------------------------------------------------
// Func: Initialization of registers for designed functionality.
// Meth: Sets up pins and timer registers for all trigger functionality to be handled
//       via ISRs.
// ------------------------------------------------------------------------------------
{
  pinMode(triggerPin, INPUT_PULLUP);
  initFireTimer();
  init56KHzCarrierFreq();
  attachInterrupt(digitalPinToInterrupt(triggerPin), triggerISR, FALLING);
}

void loop() {
  // GOAL IS TO KEEP THIS EMPTY
}

// ================================================================================EOF