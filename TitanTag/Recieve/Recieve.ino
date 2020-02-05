double timeBase = 0.6;
int freq = 56000;
int packet[] = {0,1,1,0,0};
void setup()
{
  pinMode(10,OUTPUT);
  //tone(10, freq, timeBase * 4);
  //delay(timeBase);
  for(int i = 0; i < 5; i++)
  {
    if (packet[i] == 1)
    {
      tone(10, freq, timeBase * 2);
      noTone(10);
    }
    else
    {
      tone(10, freq, timeBase);
      noTone(10);
      delay(timeBase);
    }
    delay(timeBase);
  }
}
void loop()
{
  
}

