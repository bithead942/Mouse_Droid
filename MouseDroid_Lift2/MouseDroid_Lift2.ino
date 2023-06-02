/************************************************************************
  Mouse Droid Lift Control v2
  by Bithead942

  Board:  Arduino Uno + Adafruit Motor Shield 2.3 + External Microstep driver

  Controls Lift mechanism for MSE-6 Mouse Droid

  Pins:
  SDA- I2C Motor Control (A4)
  SLC- I2C Motro Control (A5)

   0 - Serial TX
   1 - Serial RX
   2 - RC Channel for Lift trigger
   3 - Sound Card Pin to Trigger Mode Set
   4 - Grogu force move trigger
   5 - Grogu power on/off
   6 - Grogu sleep/wake
   7 - Stepper Motor 1 Frequency Adjust
   8 - Stepper Motor 1 Direction (Up/down)
   9 - Stepper Motor 1 Enable (On/Off)
   10 - Stepper Motor 2 Frequency Adjust
   11 - Stepper Motor 2 Direction (Up/down)
   12 - Stepper Motor 2 Enable (On/Off)
   13 - Sound Card Pin to Tirgger Force

*************************************************************************/
#include <Wire.h>
#include <Adafruit_MotorShield.h>

// Pin Assignments //
#define _chLiftTrigger 2   // RC Channel for Lift trigger
#define _ModeSet 3         // Sound Card Pin to Trigger Mode Set
#define _ForceTrigger 4    // Grogu force move trigger
#define _GroguPower 5      // Grogu power on/off
#define _GroguWake 6       // Grogu sleep/wake
#define _Step1Freq 7       // Stepper Motor 1 Frequency Adjust
#define _Step1Dir 8        // Stepper Motor 1 Direction (Up/down)
#define _Step1Enable 9     // Stepper Motor 1 Enable (On/Off)
#define _Step2Freq 10      // Stepper Motor 2 Frequency Adjust
#define _Step2Dir 11       // Stepper Motor 2 Direction (Up/down)
#define _Step2Enable 12    // Stepper Motor 2 Enable (On/Off)
#define _chForceTrigger 13 //DIO (from Sound board) for Force Trigger

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();

// M1 = Front Motor
Adafruit_DCMotor *FrontMotor = AFMS.getMotor(1);
// M2 = Rear Motor
Adafruit_DCMotor *BackMotor = AFMS.getMotor(2);

int LastLift = 0;
uint8_t ForceVal = 0;

void setup() {
  pinMode(_chLiftTrigger, INPUT);   //RC channel for Lift Trigger
  pinMode(_ModeSet, OUTPUT);        //DIO for Mode

  pinMode(_ForceTrigger, OUTPUT);   // Grogu force move trigger
  pinMode(_GroguPower, OUTPUT);     // Grogu power on/off
  pinMode(_GroguWake, OUTPUT);     // Grogu sleep/wake

  pinMode(_Step1Freq, OUTPUT);      //1 freq
  pinMode(_Step1Dir, OUTPUT);       //1 dir
  pinMode(_Step1Enable, OUTPUT);    //1 enable
  pinMode(_Step2Freq, OUTPUT);      //2 freq
  pinMode(_Step2Dir, OUTPUT);       //2 dir
  pinMode(_Step2Enable, OUTPUT);    //2 enable

  pinMode(_chForceTrigger, INPUT);  // DIO for Force Trigger

  digitalWrite(_ModeSet, LOW);
  LastLift = 0;
  while (LastLift == 0)
  {
  LastLift = pulseIn(_chLiftTrigger, HIGH, 25000); // RC channel for Mode
  delay(10);
  }
  //delay(40);   // required or will read next as 0

  // initialize Serial ports
  Serial.begin(9600);

  if (!AFMS.begin()) {         // create with the default frequency 1.6KHz
    Serial.println("Could not find Motor Shield. Check wiring.");
    //  while (1);
  }
  Serial.println("Motor Shield found.");

  // Set the speed to start, from 0 (off) to 255 (max speed)
  FrontMotor->setSpeed(255);
  FrontMotor->run(FORWARD);
  // turn on motor
  FrontMotor->run(RELEASE);

  // Set the speed to start, from 0 (off) to 255 (max speed)
  BackMotor->setSpeed(255);
  BackMotor->run(FORWARD);
  // turn on motor
  BackMotor->run(RELEASE);

  Serial.println("Ready.");
}

void loop() {

  int LiftTrigger = 0;
  while (LiftTrigger == 0)
  {
    LiftTrigger = pulseIn(_chLiftTrigger, HIGH, 25000); // RC channel for Lift Trigger
    delay(10);
  }
  //delay(40);   // required or will read next as 0

  if (LiftTrigger > 1500 && LastLift <= 1500)             //Lift is down, needs to come up
  {
    Serial.println("Lift up");
    GroguOn();
    LiftUp();
    LastLift = LiftTrigger;
  }
  if (LiftTrigger <= 1500 && LastLift > 1500)             //Lift is up, needs to come down
  {
    Serial.println("Lift Down");
    GroguSleep();
    LiftDown();
    GroguOff();
    LastLift = LiftTrigger;
  }

  ForceVal = digitalRead(_chForceTrigger);
  if (LastLift > 1500 && ForceVal == HIGH)        //Lift is up, trigger random animation
  {
    ActivateForce();
    delay(500);   //avoid dup
  }

  /*Serial.print("LiftTrigger: ");
    Serial.print(LiftTrigger);
    Serial.print(", ");
    Serial.print("LastLift: ");
    Serial.print(LastLift);
    Serial.print(", ");
    Serial.print("Mode: ");
    Serial.print(digitalRead(_ModeSet));
    Serial.print(", ");
    Serial.print("ForceVal: ");
    Serial.print(ForceVal);
    Serial.println(); */
  delay(10);
}

/*********************************
   Lift Up
 *********************************/
void LiftUp()
{
  digitalWrite(_ModeSet, HIGH);      //Mando Mode
  //Raise Front Lid
  FrontMotor->run(FORWARD);
  delay(500);

  //Raise Back Lid
  BackMotor->run(FORWARD);
  delay(2000);
  FrontMotor->run(RELEASE);
  BackMotor->run(RELEASE);

  //Raise Platform
  digitalWrite(_Step1Enable, LOW);   //1 enable
  digitalWrite(_Step1Dir, HIGH);     //1 up
  digitalWrite(_Step2Enable, LOW);   //2 enable
  digitalWrite(_Step2Dir, HIGH);     //2 up
  freqout(1100, 3100);               //supported frequencies:  0 - 1500

  digitalWrite(_Step1Enable, HIGH);  //1 disable
  digitalWrite(_Step2Enable, HIGH);  //2 disable
}

/*********************************
   Lift Down
 *********************************/
void LiftDown()
{
  digitalWrite(_ModeSet, LOW);       //Mouse Mode
  //Lower Platform
  digitalWrite(_Step1Enable, LOW);   //1 enable
  digitalWrite(_Step1Dir, LOW);      //1 down
  digitalWrite(_Step2Enable, LOW);   //2 enable
  digitalWrite(_Step2Dir, LOW);      //2 down
  freqout(1100, 3100);               //supported frequencies:  0 - 1500

  digitalWrite(_Step1Enable, HIGH);  //1 disable
  digitalWrite(_Step2Enable, HIGH);  //2 disable

  //Lower Back Lid
  BackMotor->run(BACKWARD);
  delay(500);

  //Lower Front Lid
  FrontMotor->run(BACKWARD);
  delay(2000);
  FrontMotor->run(RELEASE);
  BackMotor->run(RELEASE);
}

/*********************************
   Frequency Out
 *********************************/
void freqout(int freq, int t)                 // pin, freq in hz, t in ms
{
  int hperiod;                                //calculate 1/2 period in us
  long cycles, i;
  pinMode(_Step1Freq, OUTPUT);                // turn on output pin
  pinMode(_Step2Freq, OUTPUT);

  hperiod = (500000 / freq) - 7;              // subtract 7 us to make up for digitalWrite overhead

  cycles = ((long)freq * (long)t) / 1000;     // calculate cycles
  // Serial.print(freq);
  // Serial.print((char)9);                   // ascii 9 is tab - you have to coerce it to a char to work
  // Serial.print(hperiod);
  // Serial.print((char)9);
  // Serial.println(cycles);

  for (i = 0; i <= cycles; i++) {             // play note for t ms
    digitalWrite(_Step1Freq, HIGH);
    digitalWrite(_Step2Freq, HIGH);
    delayMicroseconds(hperiod);
    digitalWrite(_Step1Freq, LOW);
    digitalWrite(_Step2Freq, LOW);
    delayMicroseconds(hperiod - 1);          // - 1 to make up for digitaWrite overhead
  }
  pinMode(_Step1Freq, INPUT);                // shut off pin to avoid noise from other operations
  pinMode(_Step2Freq, INPUT);
}

/*********************************
   Activate Force
 *********************************/
void ActivateForce()
{
  for (int i = 0; i < 3; i++)
  {
    digitalWrite(_ForceTrigger, HIGH);
    delay(100);
    digitalWrite(_ForceTrigger, LOW);
    delay(200);
  }
  Serial.println("Activate");
}

/*********************************
   Grogu On
 *********************************/
void GroguOn()
{
  digitalWrite(_GroguPower, HIGH);
  Serial.println("Grogu On");
  delay(1000);
  digitalWrite(_GroguWake, HIGH);
  delay(2000);
  digitalWrite(_GroguWake, LOW);
  Serial.println("Grogu Awake");
}

/*********************************
   Grogu Sleep
 *********************************/
void GroguSleep()
{

  digitalWrite(_GroguWake, HIGH);
  Serial.println("Grogu Sleep");

}

/*********************************
   Grogu Off
 *********************************/
void GroguOff()
{

  digitalWrite(_GroguPower, LOW);
  digitalWrite(_GroguWake, LOW);
  Serial.println("Grogu Off");

}
