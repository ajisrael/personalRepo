#define TIMEBASE    9600 // Number of 16MHz clk cycles for 600us timer (bit delay)
#define TIMERMAX   65536 // Max value of 16-bit timer before overflow
#define TIMEHEADER 38400 // Number of 16MHz clk cycles for 2400us (header delay)

const int baseDelay = TIMERMAX - TIMEBASE;
const int headerDelay = TIMERMAX - TIMEHEADER;
const byte triggerPin = 2;
const byte IRLEDMASK = B00001000;
byte data = B01101001;
byte dataBit = 7;

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
  delay(200);             // Debounce delay
  TCNT4 = headerDelay;    // Delay for header of data packet
  DDRL |= IRLEDMASK;      // Set PL3 to output mode
  TIMSK4 |= (1<<TOIE4);   // enable interrupt
  TCCR4B |= (1<<CS40);    // prescaler = 1 and start timer
}

void setup() {
  // put your setup code here, to run once:
  pinMode(triggerPin, INPUT_PULLUP);
  initFireTimer();
  init56KHzCarrierFreq();
  attachInterrupt(digitalPinToInterrupt(triggerPin), fire, FALLING);
}

ISR(TIMER4_OVF_vect) {
  TCNT4 = baseDelay;            // set timer for 600us delay
  
  if (!(DDRL & IRLEDMASK))      // if last cycle was low
  {
    if (data & (1 << dataBit))  // and xmitting a 1
    {
      TCNT4 -= TIMEBASE;        // increase delay to 1200us
    }
    dataBit--;                  // increment to next bit
  }
  
  DDRL ^= IRLEDMASK;            // toggle IRLED
  
  if (dataBit == 0)             // After Xmit packet
  {
    DDRL   &= ~IRLEDMASK;       // turn off IRLED
    TIMSK4 &= ~(1<<TOIE4);      // disable interrupt
    TCCR4B &= ~(1<<CS40);       // turn off timer
    dataBit = 7;                // reset bit index
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}
