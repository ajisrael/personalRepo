//      Start of code (copy and paste into arduino sketch)
//
//                       Duino Tag release V1.01
//          Laser Tag for the arduino based on the Miles Tag Protocol.
//           By J44industries:     www.J44industries.blogspot.com
// For information on building your own Duino Tagger go to: https://www.instructables.com/member/j44/
//
// Much credit deserves to go to Duane O'Brien if it had not been for the excellent Duino Tag tutorials he wrote I would have never been able to write this code.
// Duane's tutorials are highly recommended reading in order to gain a better understanding of the arduino and IR communication. See his site http://aterribleidea.com/duino-tag-resources/
//
// This code sets out the basics for arduino based laser tag system and tries to stick to the miles tag protocol where possible.
// Miles Tag details: http://www.lasertagparts.com/mtdesign.htm
// There is much scope for expanding the capabilities of this system, and hopefully the game will continue to evolve for some time to come.
// Licence: Attribution Share Alike: Give credit where credit is due, but you can do what you like with the code.
// If you have code improvements or additions please go to http://duinotag.blogspot.com
//

#define Tolerance 300
#define PLAYER_ID 0xEF00
#define TEAM_ID 0x00C0
#define DAMAGE 0x003C
#define PARITY  0x0001

// Digital IO's
int triggerPin             = 3;      // Push button for primary fire. Low = pressed
int speakerPin             = 4;      // Direct output to piezo sounder/speaker
int audioPin               = 9;      // Audio Trigger. Can be used to set off sounds recorded in the kind of electronics you can get in greetings card that play a custom message.
int lifePin                = 6;      // An analogue output (PWM) level corresponds to remaining life. Use PWM pin: 3,5,6,9,10 or 11. Can be used to drive LED bar graphs. eg LM3914N
int ammoPin                = 5;      // An analogue output (PWM) level corresponds to remaining ammunition. Use PWM pin: 3,5,6,9,10 or 11.
int hitPin                 = 7;      // LED output pin used to indicate when the player has been hit.
int IRtransmitPin          = 2;      // Primary fire mode IR transmitter pin: Use pins 2,4,7,8,12 or 13. DO NOT USE PWM pins!! More info: http://j44industries.blogspot.com/2009/09/arduino-frequency-generation.html#more
int IRtransmit2Pin         = 8;      // Secondary fire mode IR transmitter pin:  Use pins 2,4,7,8,12 or 13. DO NOT USE PWM pins!!
int IRreceivePin           = 12;     // The pin that incoming IR signals are read from
int IRreceive2Pin          = 11;     // Allows for checking external sensors are attached as well as distinguishing between sensor locations (eg spotting head shots)
int reloadButton           = 18;     // Button to reload the gun's ammo
int reviveButton           = 19;     // Button to revive thyself
int muzzleFlash            = 22;     // LED for muzzle flash

// Minimum gun requirements: trigger, receiver, IR led, hit LED.

// Player and Game details
uint16_t myTeamID    =  1; // Team ID: Value in range of 0-3 {Red, Blue, Yellow, Green}
uint16_t myPlayerID  =  5; // Player ID: Value in range of 0-59. Players[0-49], Special weapons [50-59]
uint16_t myDamageID  =  9; // Index in damage array. Sent in IR data packet.      
uint16_t armor       =  0; // Armor for player. Each hit decrements armor by 1 and if armor is > 0 then hits are reduced by 50%.
uint16_t clipSize    = 25; // Rounds per click.         

// Feature enable and disable; 1 = enabled, 0 = disabled
bool CTRLB = 0; // Control bit for admin/refs to transmit control packets. Default = 0.
bool MZFLE = 1; // Muzzle flash enabled; Default = 1.
bool FRFRE = 0; // Frendly fire enabled; Default = 0.
bool ARMRE = 0; // Armor enabled; Default = 0.
bool CPSZE = 0; // Clip size enabled; Default = 0.

int myGameID               = 0;      // Interprited by configureGane subroutine; allows for quick change of game types.
int myWeaponID             = 0;      // Deffined by gameType and configureGame subroutine.
int myWeaponHP             = 0;      // Deffined by gameType and configureGame subroutine.
int maxAmmo                = 0;      // Deffined by gameType and configureGame subroutine.
int maxLife                = 0;      // Deffined by gameType and configureGame subroutine.
int automatic              = 1;      // Deffined by gameType and configureGame subroutine. Automatic fire 0 = Semi Auto, 1 = Fully Auto.
int deadState              = 0;
int reloadDelay            = 2;

//Incoming signal Details
int received[18];                    // Received data: received[0] = which sensor, received[1] - [17] byte1 byte2 parity (Miles Tag structure)
int check                  = 0;      // Variable used in parity checking

// Stats
volatile int ammo          = 0;      // Current ammunition
volatile int life          = 0;      // Current life

// Code Variables
int timeOut                = 0;      // Deffined in frequencyCalculations (IRpulse + 50)
volatile int FIRE                   = 0;      // 0 = don't fire, 1 = Primary Fire, 2 = Secondary Fire
volatile int TR                     = 0;      // Trigger Reading
volatile int LTR                    = 0;      // Last Trigger Reading
volatile int setupFinished          = 0;

// Signal Properties
int IRpulse                = 600;    // Basic pulse duration of 600uS MilesTag standard 4*IRpulse for header bit, 2*IRpulse for 1, 1*IRpulse for 0.
int IRfrequency            = 56;     // Frequency in kHz Standard values are: 38kHz, 40kHz. Choose dependant on your receiver characteristics
int IRt                    = 0;      // LED on time to give correct transmission frequency, calculated in setup.
int IRpulses               = 0;      // Number of oscillations needed to make a full IRpulse, calculated in setup.
int header                 = 4;      // Header lenght in pulses. 4 = Miles tag standard
int maxSPS                 = 10;     // Maximum Shots Per Seconds. Not yet used.
int TBS                    = 0;      // Time between shots. Not yet used.
int gameModeConfig [2][6] ={{1,30,30,100,100,1},{1,100,100,100,100,2}};
uint16_t damageValues [16] = {1,2,4,5,7,10,15,17,20,25,30,35,40,50,74,100};
volatile uint16_t data = 0x0000;



int myParity               = 0;      // String for storing parity of the data which gets transmitted when the player fires.



void setup() {
  // Serial coms set up to help with debugging.
  Serial.begin(9600);
  Serial.println("Startup...");
  // Pin declarations
  pinMode(triggerPin, INPUT_PULLUP);
  pinMode(speakerPin, OUTPUT);
  pinMode(audioPin, OUTPUT);
  pinMode(lifePin, OUTPUT);
  pinMode(ammoPin, OUTPUT);
  pinMode(hitPin, OUTPUT);
  pinMode(IRtransmitPin, OUTPUT);
  pinMode(IRtransmit2Pin, OUTPUT);
  pinMode(IRreceivePin, INPUT);
  pinMode(IRreceive2Pin, INPUT);
  pinMode(reloadButton,INPUT_PULLUP);
  pinMode(muzzleFlash,OUTPUT);
  pinMode(reviveButton,INPUT_PULLUP);

  
  attachInterrupt(digitalPinToInterrupt(reviveButton),revivePlayer,LOW);
  attachInterrupt(digitalPinToInterrupt(reloadButton),reloadAmmo,LOW);
  attachInterrupt(digitalPinToInterrupt(triggerPin),triggers,LOW);
  

  frequencyCalculations();   // Calculates pulse lengths etc for desired frequency
  configureGame();           // Look up and configure game details
  tagCode();                 // Based on game details etc works out the data that will be transmitted when a shot is fired

  digitalWrite(triggerPin, HIGH);      // Not really needed if your circuit has the correct pull up resistors already but doesn't harm

  for (int i = 1; i < 254; i++) { // Loop plays start up noise
    analogWrite(ammoPin, i);
    playTone((3000 - 9 * i), 2);
  }

  // Next 4 lines initialise the display LEDs
  analogWrite(ammoPin, ((int) ammo));
  analogWrite(lifePin, ((int) life));
  lifeDisplay();
  ammoDisplay();

  Serial.println("Ready....");

}


// Main loop most of the code is in the sub routines
void loop() {
  setupFinished = 1;
  receiveIR();
  if (FIRE != 0) {
    ammoDisplay();
  }
}


// SUB ROUTINES


void ammoDisplay() { // Updates Ammo LED output
  float ammoF;
  ammoF = (260 / maxAmmo) * ammo;
  if (ammoF <= 0) {
    ammoF = 0;
  }
  if (ammoF > 255) {
    ammoF = 255;
  }
  analogWrite(ammoPin, ((int) ammoF));
}


void lifeDisplay() { // Updates Ammo LED output
  float lifeF;
  lifeF = (260 / maxLife) * life;
  if (lifeF <= 0) {
    lifeF = 0;
  }
  if (lifeF > 255) {
    lifeF = 255;
  }
  analogWrite(lifePin, ((int) lifeF));
}


void revivePlayer()
  {
    digitalWrite(hitPin, LOW);
    for(int i=0;i<62;i++)
  {
    delayMicroseconds(16383);
  }
    deadState=0;
    life = maxLife;
    ammo = maxAmmo;
  }

void receiveIR() { // Void checks for an incoming signal and decodes it if it sees one.
  int error = 0;
  uint16_t receivedData = 0xFFFF;
  if (digitalRead(IRreceivePin) == LOW && deadState!=1) { 
    // If the receive pin is low a signal is being received.
//    if (digitalRead(IRreceive2Pin) == LOW) { // Is the incoming signal being received by the head sensors?
//      received[0] = 1;
//    }
//    else {
//      received[0] = 0;
//    }

    while (digitalRead(IRreceivePin) == LOW) {
    }
    for (int i = 0; i < 16; i++) {                       // Repeats several times to make sure the whole signal has been received
      received[i] = pulseIn(IRreceivePin, LOW, timeOut);  // pulseIn command waits for a pulse and then records its duration in microseconds.
    }

    for (int i = 15; i >= 0; i--) { // Looks at each one of the received pulses

      if (received[i] > (IRpulse - Tolerance) &&  received[i] < (IRpulse + Tolerance)) {
      receivedData &= ~(0x8000 >> i); // Works out from the pulse length if it was a data 1 or 0 that was received writes result to receivedTemp string
      }
      
    }
    Serial.println(receivedData,BIN);         // Print interpreted data results

    // Parity Check. Was the data received a valid signal?
   uint8_t sum = 0;
  for (uint8_t i = 1; i < 16; i++)
  {
    sum += receivedData >> i & B1;
  }
  uint16_t parityCalc = sum%2 ;
  uint16_t parityBit = receivedData & PARITY;
  check = parityBit ^ parityCalc;
    
    if (check|| receivedData == 0xFFFF) {
      error = 1;
    }
    if (error == 0) {
      Serial.println("Valid Signal");
    }
    else {
      Serial.println("ERROR");
    }
    if (error == 0) {
      interpritReceived(receivedData);
    }
  }
}


void interpritReceived(uint16_t data) { // After a message has been received by the ReceiveIR subroutine this subroutine decidedes how it should react to the data
  uint16_t playerID = ((data & PLAYER_ID)>>8);
  uint16_t teamID = ((data & TEAM_ID)>>6);
  uint16_t damageID = ((data & DAMAGE)>>2);

  Serial.print("Player ID:");
  Serial.println(playerID);
  Serial.print("Team ID:");
  Serial.println(teamID);
  Serial.print("damageID:");
  Serial.println(damageID);
  
  if(playerID != myPlayerID)
  {
    if(FRFRE||(myTeamID!=teamID))
    {
      digitalWrite(hitPin, HIGH);
      hit(damageID);
    }
  }
}


void shoot() {
  if (FIRE !=0) { 
    digitalWrite(muzzleFlash,HIGH);
    sendPulse(IRtransmitPin, 1); // Transmit Header pulse, send pulse subroutine deals with the details
    sendPulse(IRtransmitPin, 1);
    sendPulse(IRtransmitPin, 1);
    sendPulse(IRtransmitPin, 1);
    
    delayMicroseconds(IRpulse);

    for (int i = 15; i >= 0; i--) { // Transmit Byte1
      uint8_t currBit = data >> i & B1;
      if (currBit == 1) {
        sendPulse(IRtransmitPin, 1);
        //Serial.print("1 ");
      }
      //else{Serial.print("0 ");}
      sendPulse(IRtransmitPin, 1);
      delayMicroseconds(IRpulse);
    }
   digitalWrite(muzzleFlash,LOW);
  }
  FIRE = 0;
  ammo = ammo - 1;
}


void sendPulse(int pin, int len) { // importing variables like this allows for secondary fire modes etc.
  // This void genertates the carrier frequency for the information to be transmitted
  int i = 0;
  int o = 0;
  while ( i < len ) {
    i++;
    while ( o < IRpulses ) {
      o++;
      digitalWrite(pin, HIGH);
      delayMicroseconds(IRt);
      digitalWrite(pin, LOW);
      delayMicroseconds(IRt);
    }
  }
}


void triggers() {
  // Checks to see if the triggers have been presses

    if (ammo < 1) {
      FIRE = 0;
      noAmmo();
    }
   else if (life < 1) 
   {
      FIRE = 0;
      dead();
    }
    else
    {
      LTR = TR;       // Records previous state. Primary fire
      TR = digitalRead(triggerPin);      // Looks up current trigger button state
   
    
      if (TR != LTR && TR == LOW) {
        FIRE = 1;
      }
      if (TR == LOW && automatic == 1) {
        FIRE = 2;
      }

      if (FIRE == 1 || FIRE ==2) {
        
        shoot();
     for(int i=0;i<31;i++)
        {
          delayMicroseconds(16383);
        }
        // Fire rate code to be added here
      } 
 }

}


void configureGame() { // Where the game characteristics are stored, allows several game types to be recorded and you only have to change one variable (myGameID) to pick the game.
    myWeaponID = gameModeConfig[myGameID][0];
    maxAmmo = gameModeConfig[myGameID][1];
    ammo = gameModeConfig[myGameID][2];
    maxLife = gameModeConfig[myGameID][3];
    life = gameModeConfig[myGameID][4];
    myWeaponHP = gameModeConfig[myGameID][5];
}

void reloadAmmo(){
  int delayIndex = reloadDelay/0.016383;
  for(int i=0;i<delayIndex;i++)
  {
    delayMicroseconds(16383);
  }
    ammo = maxAmmo;
  }
  

void frequencyCalculations() { // Works out all the timings needed to give the correct carrier frequency for the IR signal
  IRt = (int) (500 / IRfrequency);
  IRpulses = (int) (IRpulse / (2 * IRt));
  IRt = IRt - 6;
  // Why -4 I hear you cry. It allows for the time taken for commands to be executed.
  // More info: http://j44industries.blogspot.com/2009/09/arduino-frequency-generation.html#more

  Serial.print("Oscilation time period /2: ");
  Serial.println(IRt);
  Serial.print("Pulses: ");
  Serial.println(IRpulses);
  timeOut = 4*IRpulse + 200; // Adding 50 to the expected pulse time gives a little margin for error on the pulse read time out value
}


void tagCode() { // Works out what the players tagger code (the code that is transmitted when they shoot) is
  uint16_t newData =0x0000;
  
  newData |= myPlayerID << 8;
  newData |= myTeamID <<6;
  newData |= myDamageID << 2;

  uint8_t sum = 0;
  for (uint8_t i = 0; i < 16; i++)
  {
    sum += newData >> i & B1;
  }
  if ((myParity = sum % 2) == 1)
  {
    newData |= 0x0001; 
  }
  
  // Next few lines print the full tagger code.

  data = newData;
  Serial.println("Binary:");
  Serial.print(newData,BIN);
  Serial.println("");

  Serial.print("Parity: ");
  Serial.print(myParity);
  Serial.println();
}


void playTone(int tone, int duration) { // A sub routine for playing tones like the standard arduino melody example
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(speakerPin, HIGH);
    delayMicroseconds(tone);
    digitalWrite(speakerPin, LOW);
    delayMicroseconds(tone);
  }
}


void dead() { // void determines what the tagger does when it is out of lives
  // Makes a few noises and flashes some lights
//  for (int i = 1; i < 254; i++) {
//    analogWrite(ammoPin, i);
//    playTone((1000 + 9 * i), 2);
//  }
//  analogWrite(ammoPin, ((int) ammo));
 
  analogWrite(lifePin, ((int) life));
  Serial.println("DEAD");
  digitalWrite(hitPin, HIGH);

//  for (int i = 0; i < 10; i++) {
//    analogWrite(ammoPin, 255);
//    analogWrite(ammoPin, 0);
//  }
}


void noAmmo() { // Make some noise and flash some lights when out of ammo
  digitalWrite(hitPin, HIGH);
  playTone(500, 100);
  playTone(1000, 100);
  digitalWrite(hitPin, LOW);
}


void hit(uint16_t damageIndex) { // Make some noise and flash some lights when you get shot
  life = life - damageValues[damageIndex];

  Serial.print("Life: ");
  Serial.println(life);
  playTone(500, 500);
  if (life <= 0) {
    deadState=1;
    dead();
  }
  else{
    digitalWrite(hitPin, LOW);

    }
  lifeDisplay();
}
