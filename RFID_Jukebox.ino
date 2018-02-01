/*A largely modified version of the SPOT RFID jukebox by Jean Alvin.
Original project at  http://www.mediaenlab.com 
CC License BY-NC 2.0 https://creativecommons.org/licenses/by-nc/2.0/fr/ 

Libraries
*RFID library by MiguelBalboa - https://github.com/miguelbalboa/rfid
*Grove MP3 library by SEEED studio- https://github.com/Seeed-Studio/Grove_Serial_MP3_Player_V2.0

Hardware
*Arduino/Genuino Uno
*MP3 player module: Grove MP3 v2.0 - http://wiki.seeed.cc/Grove-MP3_v2.0/
*RFID reader: MiFare MFRC522 reader - http://playground.arduino.cc/Learning/MFRC522
 
 *  Arduino/MFRC522 SPI connection
 * --------------------------------------------------------------------------------------------
 *             MFRC522      Arduino                 Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno                     Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin/broches  Pin/broches             Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9                       5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10                      53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4             51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1             50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3             52        D13        ICSP-3           15
 *


*  Arduino/MP3 Grove MP3 2.0 connection
Product page - http://wiki.seeed.cc/Grove-MP3_v2.0/
 * --------------------------------------------------------------------------------------------
 * Grove       Arduino      
 * wires       Uno
 * -----------------------------------------------------------------------------------------
 * Black       GND
 * Red         5V
 * White       3
 * Yellow      2
*/

//MP3 player setup start
#include <SoftwareSerial.h>    
SoftwareSerial mp3(2, 3);      // Declaring pins 2 + 3 for Grove MP3
#include <MP3Player_KT403A.h> //Grove MP3 2.0 library
int potPin = A0;               // Declaring A0 analog pin for pot readings
int potVal = 0;                // Pot values variable
byte mp3Vol = 0;               // Current volume level
byte oldVol = 0;               // Previous volume level
//MP3 player setup end

int button1 = 8;         //Music/commentary switch pin

int Nplaysong; //Song # storage variable

//This block is all about the EEPROM memory and jukebox usage data storage.
#include "EEPROM.h" //EEPROM memory management library
int counter;       //Counter variable
byte myHigh = highByte(counter);
byte myLow = lowByte(counter); //Combining two bytes to expand counter values past 255 and up to 1023.

//This block is all about the RFID reader.
#include <SPI.h>
#include <MFRC522.h> // RFID reader library 
#define RST_PIN    9   
#define SS_PIN    10  
MFRC522 mfrc522(SS_PIN, RST_PIN); // Creating MFRC522 instance.

void setup() {
  Serial.begin(9600);    //Initializing serial comm
  SPI.begin();          //Initializing SPI bus for RFID reader
  mfrc522.PCD_Init();   //Initializing RFID MFRC522 module
  ShowReaderDetails();  //Display RFID tag info
  Serial.println(F("Reading RFID tag to display UID, tag type and data if available"));
  
//  pinMode(button1, INPUT_PULLUP); // Declaring pin 8 input with internal pull-up
//  pinMode(5, OUTPUT); //Music LED
//  pinMode(6, OUTPUT); //Commentary LED

  counter = word(EEPROM.read(11), EEPROM.read(10)); //Serial monitor displays how many times jukebox has been used
  Serial.print("Jukebox has been used "); Serial.print (counter); Serial.println(" times");

  Nplaysong = 0; 

//Initializing Grove MP3 2.0
  delay(1000);
   mp3.begin(9600);
  SelectPlayerDevice(0x02);       // Selecting SD card as the player device.
  SetVolume(0x0E);                // Set the volume, the range is 0x00 to 0x1E.
}

void loop() {
//Replace instructions with everything needed to send instructions for the MP3 player over serial.

//val = digitalRead(button1); // setting value for "info" button. Turns on one of the two optional LEDs based on switch position
//Serial.println(digitalRead(button1));

//int sensorValue = analogRead(A0); //Keeping track of the pot value. Uncomment for diagnostic
//Serial.println(sensorValue);  //Keeping track of the pot value. Uncomment for diagnostic

  if (digitalRead(button1) == HIGH)
  {
    digitalWrite(5, HIGH);
  }
  else
  {
    digitalWrite(5, LOW);
  }

  if (digitalRead(button1) == LOW)
  {
    digitalWrite(6, HIGH);
  }
  else
  {
    digitalWrite(6, LOW);
  }

potVal = analogRead(potPin);                      // Reads pot value on A1 btwn 0 and 1023.
mp3Vol = map(potVal, 0, 1023, 0, 30);             // Maps A1 value from 0 to 1023 to 0 to 31 for volume.
if ((mp3Vol > (oldVol + 1)) || (mp3Vol < (oldVol - 1))) { // Changes volume when pot value changes
  oldVol = mp3Vol;
  delay(100);                               // Necessary serial <-> Grove MP3 communication delay
}

  // No tag? Then the loop stops here!
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  //Idem
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  //Displays the number of times jukebox was used
  if (mfrc522.uid.uidByte[5] == 0x06 || 0x49) { //This needs to be personalized to match the UIDs of the RFID tags used.
    counter++;
    //Storing counter value when RFID tag is present
    EEPROM.write(10, lowByte(counter));
    EEPROM.write(11, highByte(counter));
  }
  
  //Displaying RFID UID values in serial monitor to connect the RFID UID to the "SpecifyMusicPlay" functions
  Serial.print (mfrc522.uid.uidByte[0]);
  Serial.print (mfrc522.uid.uidByte[1]);
//  Serial.print (mfrc522.uid.uidByte[2]);
//  Serial.print (mfrc522.uid.uidByte[3]);
//Changed to fit the MiFare 1K tags

  /* "SpecifyMusicPlay" functions and how to start playing a specific track based on an RFID tag's UID.
  Each RFID UID triggers a specific track, based on the order of copying to the MP3 player SD card
  If Commentary switch function is activated and switch is engaged, we can play a commentary track
  */

//Mikal Shapiro - Daniel
if (mfrc522.uid.uidByte[0] == 0x1E && mfrc522.uid.uidByte[1] == 0xD1 && mfrc522.uid.uidByte[2]== 0x7B && mfrc522.uid.uidByte[3] == 0xAB) //Defining RFID tag identification
  {
//    ClignoteZic();  //Blinking the "Now playing" LED
    Nplaysong = 1;
    Serial.println("Mikal Shapiro - Daniel");
    SpecifyMusicPlay(1); //New play routine for Grove MP3 Player 2.0
    delay(200);
}

//Violent Bear - Long Distance Lover
if (mfrc522.uid.uidByte[0] == 0x6E && mfrc522.uid.uidByte[1] == 0xDB && mfrc522.uid.uidByte[2]== 0x7B && mfrc522.uid.uidByte[3] == 0xAB) //Defining RFID tag identification
  {
//    ClignoteZic();  //Blinking the "Now playing" LED
    Nplaysong = 3;
    Serial.println("Violent Bear - Long Distance Lover");
    SpecifyMusicPlay(3); //New play routine for Grove MP3 Player 2.0
    delay(200);
}

//The UKs - Bad Seed
if (mfrc522.uid.uidByte[0] == 0x3E && mfrc522.uid.uidByte[1] == 0xDF && mfrc522.uid.uidByte[2]== 0x7B && mfrc522.uid.uidByte[3] == 0xAB) //Defining RFID tag identification
  {
//    ClignoteZic();  //Blinking the "Now playing" LED
    Nplaysong = 5;
    Serial.println("The UKs - Bad Seed");
    SpecifyMusicPlay(5); //New play routine for Grove MP3 Player 2.0
    delay(200);
}

//Brotha Newz - Sorrow
if (mfrc522.uid.uidByte[0] == 0xBE && mfrc522.uid.uidByte[1] == 0xF6 && mfrc522.uid.uidByte[2]== 0x7B && mfrc522.uid.uidByte[3] == 0xAB) //Defining RFID tag identification
  {
//    ClignoteZic();  //Blinking the "Now playing" LED
    Nplaysong = 7;
    Serial.println("Brotha News - Sorrow");
    SpecifyMusicPlay(7); //New play routine for Grove MP3 Player 2.0
    delay(200);
}

//Tidy Hippy - Tidy Hippy EP
if (mfrc522.uid.uidByte[0] == 0xCE && mfrc522.uid.uidByte[1] == 0xCE && mfrc522.uid.uidByte[2]== 0x7B && mfrc522.uid.uidByte[3] == 0xAB) //Defining RFID tag identification
  {
//    ClignoteZic();  //Blinking the "Now playing" LED
    Nplaysong = 9;
    Serial.println("Tidy Hippy - Tidy Hippy EP");
    SpecifyMusicPlay(9); //New play routine for Grove MP3 Player 2.0
    delay(200); 
}
  
//According to Jean Alvin, Arduino Uno can hold 200 "Playsong" instructions. Will need to find an estimate for SpecifyMusicPlay(XX)!
//
// Dump debug info about the card; PICC_HaltA() is automatically called. This also prevents stutter rereads when card left on top of reader
  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
}

//Main loop ends here.

//The two following functions trigger blinking for the "Music" and "Info" LEDs based on switch position.
//void ClignoteZic() { 
//  if (digitalRead(button1) == LOW && Nplaysong != 0)
//  {
//    digitalWrite(6, LOW);
//    delay(500);
//    digitalWrite(6, HIGH);
//    delay(500);
//    digitalWrite(6, LOW);
//    delay(500);
//    digitalWrite(6, HIGH);
//    delay(500);
//  }
//}
//void ClignoteInfo() {
//  if (digitalRead(button1) == HIGH && Nplaysong != 0)
//  {
//    digitalWrite(5, LOW);
//    delay(500);
//    digitalWrite(5, HIGH);
//    delay(500);
//    digitalWrite(5, LOW);
//    delay(500);
//    digitalWrite(5, HIGH);
//    delay(500);
//  }
//}

void ShowReaderDetails() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("MFRC522 Software Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (Unknown)"));
  Serial.println("");
  // When 0x00 ou 0xFF are returned, it means we have a comm problem with the RFID module! (||= Boolean "or" )
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("Communication problem - is the RFID module plugged in?"));
  }
}

// Display info about the MP3 module in the serial monitor (optional)
void demonstrate_GET_FUNCTIONS() {
  Serial.begin(9600);
  Serial.print("Volume: ");
  Serial.print("Current state: ");
  Serial.print("Play mode : ");
  Serial.print("Number of files on SD card:");
  Serial.println("------------------------------");
}


// writeToMP3 is a generic function that aims to simplify all of the methods that control the Grove MP3 Player
void writeToMP3(byte MsgLEN, byte A, byte B, byte C, byte D, byte E, byte F) {
  byte codeMsg[] = {MsgLEN, A, B, C, D, E, F};
  mp3.write(0x7E);                        //Start Code for every command = 0x7E
  for (byte i = 0; i < MsgLEN + 1; i++) {
    mp3.write(codeMsg[i]);                //Send the rest of the command to the GROVE MP3 player
  }
}


/*Optional MP3 player control functions "just in case" - they cannot be triggered from the jukebox itself, but are available over serial
This section needs updating with the commands for the 2.0 module! Get to work Thomas!
Complete documentation - http://wiki.seeed.cc/Grove-MP3_v2.0/
Complete datasheet - https://raw.githubusercontent.com/SeeedDocument/Grove-MP3_v2.0/master/res/Grove-MP3_v2.0_KT403A_datasheet_V1.3_EN-Recompiled_by_Seeed-.pdf
*/

void setPlayMode(byte playMode) {
  /* playMode options:
        0x00 = Single song - played only once ie. not repeated.  (default)
        0x01 = Single song - cycled ie. repeats over and over.
        0x02 = All songs - cycled
        0x03 = play songs randomly                                           */
  writeToMP3(0x03, 0xA9, playMode, 0x7E, 0x00, 0x00, 0x00);
}
void playSong(byte songHbyte, byte songLbyte) {                             // Plays the selected song
  writeToMP3(0x04, 0xA0, songHbyte, songLbyte, 0x7E, 0x00, 0x00);
}
void pauseSong() {                                                          // Pauses the current song
  writeToMP3(0x02, 0xA3, 0x7E, 0x00, 0x00, 0x00, 0x00);
}
void stopSong() {                                                           // Stops the current song
  writeToMP3(0x02, 0xA4, 0x7E, 0x00, 0x00, 0x00, 0x00);
}
void playNextSong() {                                                       // Play the next song
  writeToMP3(0x02, 0xA5, 0x7E, 0x00, 0x00, 0x00, 0x00);
}
void playPreviousSong() {                                                   // Play the previous song
  writeToMP3(0x02, 0xA6, 0x7E, 0x00, 0x00, 0x00, 0x00);
}
void addSongToPlayList(byte songHbyte, byte songLbyte) {
  //Repeat this function for every song you wish to stack onto the playlist (max = 10 songs)
  writeToMP3(0x04, 0xA8, songHbyte, songLbyte, 0x7E, 0x00, 0x00);
}
void setVolume(byte Volume) {                                               // Set the volume
  byte tempVol = constrain(Volume, 0, 31);
  //Volume range = 00 (muted) to 31 (max volume) Sound output can be pretty loud, max volume can be constrained here. Adjust based on speakers/headphones.
  writeToMP3(0x03, 0xA7, tempVol, 0x7E, 0x00, 0x00, 0x00);
}
