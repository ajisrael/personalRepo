#define PEDAL A0
#define MOTOR 9
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(PEDAL, INPUT);
  pinMode(MOTOR, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int spd = analogRead(PEDAL);
  int mapped = map(spd, 50, 990, 0, 255);
  char buffer [50];
  sprintf(buffer, "%d %d\n", spd, mapped);
  Serial.print(buffer);
  analogWrite(MOTOR, mapped);
  
}
