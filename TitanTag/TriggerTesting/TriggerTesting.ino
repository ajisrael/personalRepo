#define TIME_BASE      9600 // Number of 16MHz clk cycles for 600us timer (bit delay)
#define TIMER_MAX     65536 // Max value of 16-bit timer before overflow
#define TIME_HEADER   38400 // Number of 16MHz clk cycles for 2400us (header delay)
#define PACKET_LENGTH     2 // Default length of the packet being sent in bytes

const int baseDelay = TIMER_MAX - TIME_BASE;
const int headerDelay = TIMER_MAX - TIME_HEADER;
const byte triggerPin = 2;
const byte IRLEDMASK = B00001000;
const byte MSB_Byte = B10000000;
int  triggerDelay = 100000;            // Delay for 10 ms
byte packetLength = PACKET_LENGTH;     // The current length of the packet
byte data[2] = {B01101001, B11110000}; // Data to send
byte dataBit = MSB_Byte;               // Index of which data bit
byte dataByte = 0;     // Index of which data byte

void initFireTimer() 
// ------------------------------------------------------------------------------------
// Func: Initializes the registers for Timer 4 to alternate the mode of OC5A in the
//       TIMER4_OVF ISR. This will then be used to encode the data for the IR signal.
// Meth: Turns off the timer and initializes the counter. Also makes sure PL3 (pin 46)
//       is set to input mode to prevent misfire.
// Defn: TCCR4A = Timer/Counter4 Control Register A
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
// Defn: TCCR5A = Timer/Counter5 Control Register A
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

void fire() 
{ 
  delayMicroseconds(triggerDelay);
  TCNT4 = headerDelay;    // Delay for header of data packet
  DDRL |= IRLEDMASK;      // Set PL3 to output mode
  TIMSK4 |= (1<<TOIE4);   // enable interrupt
  TCCR4B |= (1<<CS40);    // prescaler = 1 and start timer
}

ISR(TIMER4_OVF_vect) {
  if (dataBit == 0)             // After Xmit packet
  {
    dataBit = MSB_Byte;         // reset bit index
    packetLength--;             // Mark 1 byte sent
    dataByte++;
    if (packetLength == 0)
    {
      DDRL   |=  IRLEDMASK;       // Set IRLED high for toggle off
      TIMSK4 &= ~(1<<TOIE4);      // disable interrupt
      TCCR4B &= ~(1<<CS40);       // turn off timer
      packetLength = PACKET_LENGTH; // reset packet length
      dataByte = 0;
      Serial.println("complete");
    }
  }
  
  TCNT4 = baseDelay;            // set timer for 600us delay
  
  if (!(DDRL & IRLEDMASK))      // if last cycle was low
  {
    if (data[dataByte] & dataBit)  // and xmitting a 1
    {
      TCNT4 -= TIME_BASE;       // increase delay to 1200us
    }
    dataBit /= 2;     // increment to next bit
  }
  
  DDRL ^= IRLEDMASK;            // toggle IRLED
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(triggerPin, INPUT_PULLUP);
  initFireTimer();
  init56KHzCarrierFreq();
  attachInterrupt(digitalPinToInterrupt(triggerPin), fire, FALLING);
}

void loop() {
  // put your main code here, to run repeatedly:
}
