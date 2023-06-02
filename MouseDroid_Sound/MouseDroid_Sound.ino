/************************************************************************
  Mouse Droid Sound for HotRC D600
  by Bithead942

  Board:  Arduino Pro Mini

  Controls Sound for MSE-6 Mouse Droid

  Pins:
   0 - Serial TX
   1 - Serial RX
   2 - Force Trigger (Motor Controller)
   3 - Adafruit Sound FX Board - Reset Pin
   4 - Adafruit Sound FX Board - TX Pin
   5 - Adafruit Sound FX Board - RX Pin
   6 - RC Channel - Trigger short audio
   7 - Set Mode (Motor Controller)
   8 - RC Channel - Trigger long audio loop
   9 - RC Channel - Trigger Music

*************************************************************************/

#include <SoftwareSerial.h>
#include "Adafruit_Soundboard.h"

#define _chSoundTrigger 6
#define _chModeSet 7
#define _chSoundLong 8
#define _chMusicTrigger 9
#define _chForceTrigger 2

// Choose any two pins that can be used with SoftwareSerial to RX & TX
#define SFX_TX 4
#define SFX_RX 5

// Connect to the RST pin on the Sound Board
#define SFX_RST 3

uint8_t LastMode = 0;
uint8_t LastSound = 0;
int LastSoundLong = 0;
int LastMusicTrigger = 0;

// Software Serial Communication to SFX Board
SoftwareSerial ss = SoftwareSerial(SFX_TX, SFX_RX);

// pass the software serial to Adafruit_soundboard, the second
// argument is the debug port (not used really) and the third
// arg is the reset pin
Adafruit_Soundboard sfx = Adafruit_Soundboard(&ss, NULL, SFX_RST);

void setup() {
  pinMode(_chSoundTrigger, INPUT);    //RC channel for Sound Trigger
  pinMode(_chModeSet, INPUT);         //DIO for Mode
  pinMode(_chSoundLong, INPUT);       //RC channel for Long Sound Loop
  pinMode(_chMusicTrigger, INPUT);    //RC channel for Music Trigger
  pinMode(_chForceTrigger, OUTPUT);   //DIO for Force Trigger

  digitalWrite(_chForceTrigger, LOW); //Default off
  LastMode = LOW;

  while (LastSoundLong == 0)
  {
  LastSoundLong = pulseIn(_chSoundLong, HIGH, 25000); // RC channel for Mode
  delay(10);
  }
  while (LastMusicTrigger == 0)
  {
  LastMusicTrigger = pulseIn(_chMusicTrigger, HIGH, 25000); // RC channel for Mode
  delay(10);
  }

  // initialize Serial ports
  Serial.begin(9600);
  // softwareserial at 9600 baud
  ss.begin(9600);

  Serial.println("Ready.");
  sfx.playTrack((uint8_t)0);  //play Micky Mouse
}

void loop() {

  int SoundTrigger = 0;
  while (SoundTrigger <= 500)  //loop until we get a good value
  {
    SoundTrigger = pulseIn(_chSoundTrigger, HIGH, 25000); // RC channel for Sound Trigger
    delay(10);
  }
  if (SoundTrigger > 1500) {
    Serial.println("Playing Audio");
    PlayAudio();
  }

  uint8_t ModeVal = digitalRead(_chModeSet);
  if (ModeVal != LastMode) {
    Serial.println("Switching Modes");
    ChangeMode(ModeVal);
    delay(1000);
  }

  int SoundLong = 0;
  while (SoundLong <= 500)  //loop until we get a good value
  {
    SoundLong = pulseIn(_chSoundLong, HIGH, 25000); // RC channel for Mode
    delay(10);
  }
  if ((SoundLong > 1500 && LastSoundLong <= 1500) || (SoundLong <= 1500 && LastSoundLong > 1500)) {
    Serial.println("Playing Long Sound");
    PlayLongSound();
    LastSoundLong = SoundLong;
  }

  int MusicTrigger = 0;
  while (MusicTrigger <= 500)  //loop until we get a good value
  {
    MusicTrigger = pulseIn(_chMusicTrigger, HIGH, 25000); // RC channel for Mode
    delay(10);
  }
  if ((MusicTrigger > 1500 && LastMusicTrigger <= 1500) || (MusicTrigger <= 1500 && LastMusicTrigger > 1500)) {
    Serial.println("Playing Music");
    PlaySong();
    LastMusicTrigger = MusicTrigger;
  }
  
  Serial.print("SoundTrigger: ");
  Serial.print(SoundTrigger);
  Serial.print(", ");
  Serial.print("Mode: ");
  Serial.print(ModeVal);
  Serial.print(", ");
  Serial.print("LastMode: ");
  Serial.print(LastMode);
  Serial.print(", ");
  Serial.print("SoundLong: ");
  Serial.print(SoundLong);
  Serial.print(", ");
  Serial.print("LastSoundLong: ");
  Serial.print(LastSoundLong);
  Serial.print(", ");
  Serial.print("MusicTrigger: ");
  Serial.print(MusicTrigger);
  Serial.print(", ");
  Serial.print("LastMusicTrigger: ");
  Serial.println(LastMusicTrigger); 

}

/*********************************
   Play Sound
 *********************************/
void PlayAudio()
{
  uint8_t x = LastSound;

  if (LastMode == 0)  //Mouse Mode
  {
    while (x == LastSound) 
    {
      x = random(3, 17); // start (inclusive), end (exclusive)
    }
  }
  else                //Mando Mode
  {
    while (x == LastSound) 
    {
      x = random(18, 28); // start (inclusive), end (exclusive)
    }
  }
  LastSound = x;
  sfx.playTrack(x);
  delay(2000);  //Let audio play before checking button press again
}

/*********************************
   Play Long Sound
 *********************************/
void PlayLongSound()
{
  if (LastMode == 0)  //Mouse Mode
  {
    sfx.playTrack((uint8_t) 1);  
    delay(3000);
  }
  else                //Mando Mode
  {
    digitalWrite(_chForceTrigger, HIGH);  //Trigger Force
    delay(900);
    digitalWrite(_chForceTrigger, LOW);
  }
}

/*********************************
   Play Song
 *********************************/
void PlaySong()
{
  if (LastMode == LOW) {         //Mouse Mode
    sfx.playTrack((uint8_t)2);
  }
  else {
    sfx.playTrack((uint8_t)17);  //Mando Theme
  }
  delay(3000);
}

/*********************************
   Change Mode
 *********************************/
void ChangeMode(int ModeVal)
{
  if (ModeVal == LOW) {  //Mouse Mode
    sfx.playTrack((uint8_t)3);  //play sound
    LastMode = ModeVal;
  }
  else {                //Mando Mode
    sfx.playTrack((uint8_t)17);  //Mando Theme
    LastMode = ModeVal;
  }
}
