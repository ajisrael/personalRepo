void setup() {
  // put your setup code here, to run once:
  init56KHzCarrierFreq();
}

void init56KHzCarrierFreq() {
  // ----------------------------------------------------------------------------------
  // Func: Initializes the registers for Timer 5 to generate a 56KHz PWM signal with a
  //       50% duty cycle. This will be the carrier frequency for the IR.
  // Meth: Sets the timer to Fast PWM mode to set OC5A (pin 46) high on a match and 
  //       reset when ICR5 is met. The values were calculated as 35 for ICR5 and 17 for 
  //       OCR5A as shown in their respective comments.
  // Defn: TCCR5A = Timer/Counter5 Control Register A
  //       TCCR5B = Timer/Counter5 Control Register B
  //       ICR5   = Input Compare Register 5
  //       OCR5A  = Output Compare Register 5A
  // ----------------------------------------------------------------------------------
  pinMode(46,OUTPUT);
  TCCR5A = 0; // Clear timer registers
  TCCR5B = 0; 
  TCCR5A |= (1<<WGM11) | (1<<COM5A1) | (1<<COM5B1); // Fast PWM, Set OC5A high on match
  TCCR5B |=  (1<<CS11) |  (1<<WGM12) |  (1<<WGM13); // prescaler 8,  Fast PWM IRC5 Top
  ICR5 = 35;  // 16MHz / 8 = 2MHz / 56KHz = 35.7 -> 36 - 1 (0 index) = 35
  OCR5A = 17; // 36 / 2 - 1 (0 index) = 17
}

void loop() {
  // put your main code here, to run repeatedly:
  
}
