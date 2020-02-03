int val = 0;
void setup() {
  // put your setup code here, to run once:
  pinMode(11, INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  val = digitalRead(11);
  Serial.println(val);
}