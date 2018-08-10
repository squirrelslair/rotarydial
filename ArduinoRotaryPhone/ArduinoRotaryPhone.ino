/* rotary phone answerer

 This will 
 - run a dial tone
 - wait for dialling to start to stop playing it
 - wait for a phone number
 - if number is right, play a message 
 - if number is wrong, play a busy signal

 pin dial_on_pin is hooked to the Nortel dial's on-when-dialling wire
 pin count_pin is hooked to the Nortel dial's counter wire
 pin cradle_pin is hooked to the on-when-picked-up wire
 */

#include "SparkFun_WT2003S_MP3_Decoder.h" // Click here to get the library: http://librarymanager/All#SparkFun_WT2003S

//--------------------------------------------------------------
// Set-up code for WT2003 MP3 Decoder
#define DIAL_TONE 3
#define BUSY_SIGNAL 2
#define MSG 1

SoftwareSerial mp3_sws(2, 3);    // Use software serial so that the Arduino hardware serial can be used for control commands

WT2003S MP3;                     // Create an object of the WT2003S class called MP3

//--------------------------------------------------------------
// Setup hardware definitions for phone
#define dial_on_pin 5
#define count_pin 4
#define cradle_pin 6

#define debg false
#define targetPhoneNr "5156273" //nr that needs to be called to trigger msg --------------------------------------------------------------

void setup() {
  
  if (debg) Serial.begin(9600);
  while (!Serial) {};
  MP3.begin(mp3_sws);      // Beginning the MP3 player requires a serial port (either hardware or software!!!)
 
  pinMode(dial_on_pin, INPUT_PULLUP); // set up the pin to be true when grounded
  pinMode(count_pin, INPUT_PULLUP);   // set up the pin to be true when grounded
  pinMode(cradle_pin, INPUT_PULLUP);  // set up the pin to be true when grounded

  MP3.setVolume(31); //max is 31, the speaker in handset seems to need all of that
 
  debugPartMsg ("dial ");
  debugPartMsg (targetPhoneNr);
  debugMsg (" to have phone play msg");

  // wait for an initial cradle-down to have a consistent starting point
  if (offHook()){
     debugMsg ("on boot, phone is off hook, needs to be put down"); 
     waitForHangup(); // don't proceed until hung up
     stopPlayback();
     debugMsg ("hung up");
  } 
  else {
     debugMsg ("on boot, phone is hung up"); 
  }
}

void loop() {
  //TestInputs(); 
  BeAPhone ();
}

void BeAPhone ( void ){
    String phoneNr = "";
    uint8_t MP3_result = 0;
    int dialledDigit;
    String targetNr = targetPhoneNr;

    debugMsg ("hung up, starting to wait for pickUp");
    
    while (hungUp()){   }
    debugMsg ("phone taken off hook");
    playDial();
  
    while (offHook()){ 

        debugMsg ("top of dial loop");

        dialledDigit = get_dialler_digit();
        debugMsg(String(dialledDigit));
        
        if (dialledDigit < 0) {
          debugMsg ("dialledDigit returned -1, hang-up while waiting for dialing");
          stopPlayback(); 
          break; 
        } 
        
        phoneNr += String(dialledDigit); 
        debugMsg ("dialledDigit returned a >0 number");

        if (phoneNr == targetNr){
          debugMsg ("Yay you called Tina, waiting for hangup");
          playMsg(); 
          waitForHangup();  //if not, will not proceed as that function is a loop
          debugMsg ("hangup after right number");
          stopPlayback(); 
          phoneNr = ""; 
          break;
        }
        
        else if (phoneNr.length() >= targetNr.length()){ //wrong number was dialled
          debugMsg("Wrong number, playing busy signal, waiting for hangup");
          playBusy();
          waitForHangup();  //if not, will not proceed as that function is a loop
          stopPlayback(); 
          phoneNr = ""; 
          break;
        }

        // if we got this far without break then get next numebr
        debugMsg ("keep dialling");

    }  
}
int get_dialler_digit( void ) {
    int c = 0;
    
    while (not nowDialling()) {
        // if hangup, then return -1 as in no number as in error
        if (hungUp()) {
          debugMsg ("hung up during dial, returning -1");
          return -1;
        }
    } // wait for dialling to start

    // dialling starts, stop the dial tone
    stopPlayback(); 
 
    if (nowDialling()){debugMsg("dial now active, dial tone stops");}
    
    while (nowDialling()){
      
        while (digitalRead(count_pin) == 0) {} //wait until turns true

        debugPartMsg ("count signal detected, count was "); // partial line that's why not debugmsg
        delay(5); //occasional overcount without delay
        debugMsg (String(c));
        
        while (digitalRead(count_pin) == 1) {} //wait until false again
        
        c++;
        
        debugPartMsg ("count is now ");  // partial line that's why not debugmsg
        delay(5); //occasional overcount without delay
        debugMsg (String(c));  
        delay (20); //let the dial_on finish if it will
    }
    
    debugMsg ("dial no longer active");

    if (c == 10) {c = 0;} //0 makes 10 clicks
    return c;
}
void waitForHangup( void ){
  debugMsg ("function waitForHangup starts waiting");  
  while (offHook()){
     // wait while off hook
  }
  debugMsg ("function waitForHangup detects hangup");  
}
boolean hungUp( void ){
  return (digitalRead(cradle_pin) == 1);
}
void waitForPickup( void ){
  debugMsg ("function waitForPickup starts waiting");  
  while (hungUp()){
     // wait while hung up
  }
  debugMsg ("function waitForPickup detects pickup");
}
boolean offHook( void ){
  return (digitalRead(cradle_pin) == 0);
}
boolean nowDialling( void ){
  return (digitalRead(dial_on_pin) == 0);
}
void debugMsg(String s){
  if (debg) Serial.println(s);
}
void debugPartMsg(String s) { 
  if (debg) Serial.print(s); 
}

void TestInputs ( void ) {
   Serial.print("pin4: ");
   Serial.print(digitalRead(4));
   Serial.print("   pin5: ");
   Serial.print(digitalRead(5));
   Serial.print("   pin6: ");
   Serial.print(digitalRead(6));
   Serial.println("");
}

//-----------------------------------------------------------------------------
// MP3 functions
void playDial ( void ) {
  MP3.setPlaymodeSingleLoop();
  MP3.playTrackNumber(DIAL_TONE);
}
void playBusy ( void ) {
  MP3.setPlaymodeSingleLoop();
  MP3.playTrackNumber(BUSY_SIGNAL);
}
void playMsg ( void ) {
  MP3.setPlaymodeSingleNoLoop();
  MP3.playTrackNumber(MSG); 
}
void stopPlayback ( void ) {
  MP3.stopPlaying();
}

