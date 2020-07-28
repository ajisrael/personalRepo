// ================================================================================BOF
// Orig: 2020.06.02 - Alex Israels
//
// Func: Sends and receives data via infrared when a trigger is pressed all using 
//       interrupts so the main loop is left empty.
//
// Meth: Timer/Counter5 creates a 56KHz carrier frequency for the IR signal tied to
//       pin 46 (OC5A). Timer/Counter4 is used to send the encoded data when the
//       trigger button is pressed by changing the mode of pin 46 to allow the
//       carrier frequency signal through for a specific ammount of time for a 1 or 0.
//       Timer/Counter3 is used for parsing the input from a recieved packet. This
//       is done using a high sample rate to determine the value accross the time base
//       similar to how a UART receiver functions. The data is then saved in a buffer
//       which is then analyzed to adjust a players variables.
//
// Defn: TIME_BASE     = # of 16MHz clk cycles for Timer4 to overflow after 600us
//       TIMER_MAX     = Maximum value of 16-bit timers
//       TIME_HEADER   = # of 16MHz clk cycles for Timer4 to overflow after 2400us
//       XMIT_LENGTH   = Default length of the packet transmitted in bytes
//       RECV_LENGTH   = Default length of the packet received in bytes
//       DELAY_10MS    = Delay value of 10 ms in us
//       SAMPLE_AMT    = Number of samples to take per time base when recv
//       RECV_2DELIM   = currBit value when two delimitters are received b2b
//       RECV_ZERO     = currBit value when a 0 is received
//       RECV_ONE      = currBit value when a 1 is received
//       RECV_UNDEF    = Should never receive a delimiter after 3 time bases w/o one
//       RECV_HEADER   = urrBit value when parsing the header of a packet
//
// Cont: IRLEDMASK     = Mask for the IRLED pin in DDRL (Data Direction Reg L)
//       MSB_Byte      = Mask for most significant bit of a byte
//       triggerPin    = The pin attached to the trigger (must be a HW interrupt pin)
//       baseDelay     = Base delay of 600us for Timer4
//       headerDelay   = Header delay of 2400 us for Timer4
//       sampleDelay   = Delay to sample SAMPLE_AMT times per time base
//
// Vars: triggerDelay  = Delay for trigger to prevent spamming
//       xmitPktLength = The current length of the transmit packet
//       xmitPacket    = Buffer for data to transmit
//       xmitBit       = Index of which bit of current byte in data array to transmit
//       xmitByte      = Index of which byte of data array to transmit
//       recvPktLength = The current length of the receive packet in bytes
//       recvPacket    = Buffer to hold received data
//       recvByte      = Index of byte in receiver buffer
//       recvBit       = Index of bit of byte in receiver buffer
//       countLow      = Weight for nummber of 0's sampled in a single time base
//       countHigh     = Weight for nummber of 1's sampled in a single time base
//       sampleCount   = Counter for number of samples taken in a single time base
//       sampleValue   = Sampled digital value from receiver
//       currBit       = Current bit value received
//       currByte      = Current byte value received
//       headerRecv    = Boolean for marking when/if a header was received from packet
//
// Rev:  2020.06.05 - Alex Israels:
//                  - Integrated timer4 to control output of IRLED pin with 600 us
//                    time base and variable duty cycle to correctly transmit data.
//       2020.06.08 - Alex Israels:
//                  - Added capability to T4_OVF_ISR to be able to send multiple
//                    bytes of data.
//       2020.06.09 - Alex Israels:
//                  - Added comments and documentation.
//       2020.07.04 - Alex Israels:
//                  - Developed preliminary receiver logic that hasn't been tested
//                    with hardware. 
//                  - Moved code to new "Ver0_1" document to save old code in event
//                    something were to break with new changes.
//                  - Added additional comments to document new changes.
//                  - Renamed variables for transmitted data.
//       2020.07.21 - Alex Israels:
//                  - Testing new recieve functions, not working.
//-----------------------------------------------------------------------------------

// Definitions
#define TIME_BASE      9600 // Number of 16MHz clk cycles for 600us timer (bit delay)
#define TIMER_MAX     65536 // Max value of 16-bit timer before overflow
#define TIME_HEADER   38400 // Number of 16MHz clk cycles for 2400us (header delay)
#define XMIT_LENGTH       4 // Default length of the packet being sent in bytes
#define RECV_LENGTH       4 // Default length of the packet received in bytes
#define DELAY_10MS   100000 // Delay value of 10 ms in us
#define SAMPLE_AMT        8 // Number of samples to take per time base when recv
#define RECV_2DELIM       0 // currBit value when two delimitters are received b2b
#define RECV_ZERO         1 // currBit value when a 0 is received
#define RECV_ONE          2 // currBit value when a 1 is received
#define RECV_UNDEF        3 // Should never receive a delimiter after 3 time bases
#define RECV_HEADER       4 // currBit value when parsing the header of a packet

// Byte masks
const byte IRLEDMASK = B00001000;      // Mask for the IRLED pin in DDRL
const byte MSB_Byte = B10000000;       // Mask for most significant bit of a byte

// Pin definitions
const byte triggerPin  = 2;            // Pin attached to trigger
const byte receiverPin = 3;            // Pin attached to receiver(s)

// Fixed delay variables
const int baseDelay = TIMER_MAX - TIME_BASE;     // Base delay of 600 us for timer4
const int headerDelay = TIMER_MAX - TIME_HEADER; // Header delay of 2400 us for timer4
const int sampleDelay = TIMER_MAX - (TIME_BASE / SAMPLE_AMT); // Delay btw each sample

// Flexible delay variables
int  triggerDelay = DELAY_10MS;                    // Delay for trigger (no spamming)

// Transmit Packet variables
byte xmitPktLength = XMIT_LENGTH;                  // The current length of the packet
byte xmitPacket[XMIT_LENGTH] = {'*','*','7','*'};  // Transmit buffer
volatile byte xmitBit = MSB_Byte;                  // Index of which data bit
volatile byte xmitByte = 0;                        // Index of which data byte

// Receive Packet variables
byte recvPktLength = RECV_LENGTH;                  // The current length of the packet
byte recvPacket[RECV_LENGTH] = {0, 0, 0, 0};       // Receive buffer
volatile byte recvByte = 0;                        // Index of byte in recv buffer
volatile byte recvBit = MSB_Byte;                  // Index of bit of byte in buffer
volatile byte countLow = 0;                        // Weight for nummber of 0's sampled
volatile byte countHigh = 0;                       // Weight for nummber of 1's sampled
volatile byte sampleCount = 0;                     // Counter for # of samples taken
volatile byte sampleValue = 0;                     // Sampled value from receiver
volatile byte currBit = 0;                         // Current bit value from receiver
volatile byte currByte = 0;                        // Current byte value received
volatile bool headerRecv = 0;                      // Boolean for checking for header

void initXmitTimer() 
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

void initRecvTimer() 
// ------------------------------------------------------------------------------------
// Func: Initializes the registers for Timer 3 to help take multiple samples of the 
//       received data stream when receiving a shot.
// Meth: Turns off the timer.
// Refs: TCCR3A = Timer/Counter3 Control Register A
//       TCCR3B = Timer/Counter3 Control Register B
// ------------------------------------------------------------------------------------
{
  TCCR3A = 0;     // clear registers and turn timer off
  TCCR3B = 0;
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
  if (xmitBit == 0)                 // After Xmit byte complete
  {
    xmitBit = MSB_Byte;             // reset bit index
    xmitPktLength--;                 // Mark 1 byte sent
    xmitByte++;                     // increment to next byte

    if (xmitPktLength == 0)          // Check for end of packet
    {
      DDRL   |=  IRLEDMASK;         // Set IRLED high for toggle off
      TIMSK4 &= ~(1<<TOIE4);        // disable interrupt
      TCCR4B &= ~(1<<CS40);         // turn off timer
      xmitPktLength = XMIT_LENGTH; // reset packet length
      xmitByte = 0;                 // reset byte index
    }
  }
  
  TCNT4 = baseDelay;                // set timer for 600us delay
  
  if (!(DDRL & IRLEDMASK))          // if last cycle was low
  {
    if (xmitPacket[xmitByte] & xmitBit)   // and xmitting a 1
    {
      TCNT4 -= TIME_BASE;           // increase delay to 1200us
    }
    xmitBit /= 2;                   // increment to next bit
  }
  
  DDRL ^= IRLEDMASK;                // toggle IRLED
}

void resetReceiver()
// ------------------------------------------------------------------------------------
// Func: Resets the varuables used to process a received packet without altering the
//       contents of the receive buffer.
// Meth: Clears the timer registers to turn off the timer. Then sets the correct delay
//       value for the counter TCNT3 to begin sampling when the receiver gets the next
//       packet. Then all the other variables used for processing a received packet are
//       reset to their default values at startup.
// Refs: TCNT3  = Timer/Counter3 counter register value
//       TCCR3A = Timer/Counter3 Control Register A
//       TCCR3B = Timer/Counter3 Control Register B
// ------------------------------------------------------------------------------------
{
    // reset timer registers
    TCCR3A = 0;             // clear registers and turn timer off
    TCCR3B = 0;
    TCNT3 = sampleDelay;    // Delay for sampling when receiving a packet

    // reset receiver variables to default values
    recvByte = 0;                                 // Index of byte in recv buffer
    recvBit = MSB_Byte;                           // Index of bit of byte in buffer
    countLow = 0;                                 // Weight for nummber of 0's sampled
    countHigh = 0;                                // Weight for nummber of 1's sampled
    sampleCount = 0;                              // Counter for # of samples taken
    sampleValue = 0;                              // Sampled value from receiver
    currBit = 0;                                  // Current bit value from receiver
    currByte = 0;                                 // Current byte value received
    headerRecv = 0;                               // Boolean for checking for header

    // Make sure the interrupt is enabled
    attachInterrupt(digitalPinToInterrupt(receiverPin), receiverISR, FALLING);
}

void clearReceiver()
// ------------------------------------------------------------------------------------
// Func: Resets the receiver and clears the contents of the receive buffer.
// Meth: Calls resetReceiver function and loops over each byte in the receivePacket
//       buffer, setting them all to zero.
// ------------------------------------------------------------------------------------
{
    resetReceiver(); // Get receiver ready for next packet

    // Clear the data in the receive buffer
    for (int i = 0; i < recvPktLength; i++) { recvPacket[i] = 0; }
}

void receiverISR()
// ------------------------------------------------------------------------------------
// Func: Gets the receiver ready for processing the incomming packet.
// Meth: Clears the receiver buffer and resets all receiver variables to default values
//       using the clearReceiver function. Then sets the correct delay for sampling the
//       received data stream and starts timer3.
// Refs: TCNT3  = Timer/Counter3 counter register value
//       TIMSK3 = Timer/Counter3 interrupt enable mask
//       TCCR3B = Timer/Counter3 Control Register B
// ------------------------------------------------------------------------------------
{
  Serial.println("Received");
  clearReceiver();        // Clear the receiver
  TCNT3 = sampleDelay;    // Delay for sampling when receiving a packet
  TIMSK3 |= (1<<TOIE3);   // enable interrupt
  TCCR3B |= (1<<CS40);    // prescaler = 1 and start timer
  detachInterrupt(digitalPinToInterrupt(receiverPin)); // Disable recieverISR()
}

ISR(TIMER3_OVF_vect)
// ------------------------------------------------------------------------------------
// Func: ISR for timer 3 overflow. Sets the delay to sample each bit at it comes in 
//       according to SAMPLE_AMT. After aquiring samples the average is calculated to
//       determine the bit value and the next bit is sampled.
// Meth: Checks for if the packet has been completely received and resets index 
//       variables if complete. When not complete, it checks the current value from the
//       receiver and increments the correct count variable. For example, if a value of 
//       1 was returned from the receiver then the countHigh variable is incremented. 
//       Once SAMPLE_AMT of checks have been recorded the maximum count variable is
//       used to determine if a delimitter or numerical value is being assessed. If it 
//       is a delimitter the previous bit value has been received and is added to the 
//       current byte. If we are evaluating a 1 or 0 the number of time bases evaluated
//       is incremented to be used when a delimiter comes along to add the byte. 
//       Once a byte is complete the index variables are incremented and the next byte 
//       is received until complete. If the number of time bases exceeds 4 then a 
//       packet has finished being processed.
// ------------------------------------------------------------------------------------
{
    sampleValue = digitalRead(receiverPin); // Get current value of receiver
    if (sampleValue) { countHigh++; } else { countLow++; } // Increment weight counters
    sampleCount++;                          // Increment number of samples taken

    if (sampleCount == SAMPLE_AMT) // Once samples are taken for current time base
    {
        sampleCount = 0;           // Reset sample counter for next time base
        if (countHigh <= countLow) // Evaluating a high vs low portion of signal
        {                          // High is delimitter, low is data
            currBit++;             // Increment to determine value of current bit
        }
        else                       // Evaluating data
        {
          Serial.print("\nEvaluating Data: ");
            switch (currBit)       // Begin parsing data
            {
                case RECV_2DELIM:     // 2 delimitters in a row (shouldn't happen)
                  Serial.print("Error: 2 Delimitters ");
                  clearReceiver();  // Handle error by clearing receiver
                case RECV_ZERO:              // 1 time base indicates a received 0
                  Serial.print("0, ");
                  if (headerRecv)          // Make sure header packet received
                  {
                      recvBit /= 2;        // increment to next bit
                  }
                case RECV_ONE:               // 2 time bases indicates a received 1
                  Serial.print("1, ");
                  if (headerRecv)          // Make sure header packet received
                  {
                      currByte |= recvBit; // Save the 1 in the current byte
                      recvBit /= 2;        // increment to next bit
                  }
                case RECV_UNDEF:      // 3 time bases btw delims (shouldn't happen)
                  Serial.print("Error: 3 Time Bases, Undefined ");
                  clearReceiver();  // Handle error by clearing receiver
                case RECV_HEADER:     // 4 time bases indicates a header
                  Serial.print("Header Recieved: ");
                  headerRecv = 1;
                default:              // 5 or greater indicates end of packet
                  Serial.print("End of Packet ");
                  resetReceiver();  // Get ready to receive next packet
            }

            currBit = 0;      // Reset value for current bit
            
            if (recvBit == 0) // Check if a full byte has been processed
            {
                Serial.print("Byte Processed: ");
                recvPacket[recvByte] = currByte; // Save value of byte to buffer
                recvByte++;                      // Increment to next byte
                currByte = 0;                    // Reset the current byte
            }

            if (recvByte == recvPktLength) // Check if packet length has been processed
            {
                Serial.print("Packet Recieved:");
                //for (int i = 0; i < recvPktLength; i++) { Serial.print(recvPacket[i], HEX); }
                resetReceiver();           // Get ready to receive next packet
            }
        }
        countHigh = 0;    // Reset value for counters
        countLow = 0;  
    }

    TCNT3 = sampleDelay;      // Reset timer for sample delay time
}

void setup()
// ------------------------------------------------------------------------------------
// Func: Initialization of registers for designed functionality.
// Meth: Sets up pins and timer registers for all functionality to be handled via ISRs.
// ------------------------------------------------------------------------------------
{
  Serial.begin(115200);
  pinMode(triggerPin, INPUT_PULLUP);
  initXmitTimer();
  initRecvTimer();
  init56KHzCarrierFreq();
  attachInterrupt(digitalPinToInterrupt(triggerPin), triggerISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(receiverPin), receiverISR, FALLING);
}

void loop() {
  // GOAL IS TO KEEP THIS EMPTY
}

// ================================================================================EOF
