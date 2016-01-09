/*
 * This is the code generator part of This Is Not A Wherigo
 * The Hardware is a LabiDuino ("self"-made clone). In Arduino 1..0.3 the boardtype is "nano with 328". In 1.6.x it's "nano" and "processor" is Atmega328.
 * 
 * 
 * 
 */
#include <LiquidCrystal_I2C.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>
// #include <EEPROM.h>
#include <Time.h>
#include <Wire.h>
#include <VirtualWire.h>

//LiquidCrystal_I2C lcd(0x20,4,5,6,0,1,2,3,7); 
LiquidCrystal_I2C lcd(0x20,4,5,6,0,1,2,3,7,NEGATIVE); 

/*
#include <MemoryFree.h>
#include <pgmStrToRAM.h>
*/

// 433-stuff
int txpowerPin = 10; // we turn the 433-module on by setting this PIN HIGH
int RF_TX_PIN = 9; // transmit PIN

/* GPS-related variables */
float flat, flon;
unsigned long age, date, time;
int Year;
byte Month, Day, Hour, Minute, Second;

long int start; // timer for letting the gps-object 'feed' more than once

// for the rotating part of the display
int displayState = 0;

TinyGPS gps;
// rx = 6 (yellow), tx = 7 (white)
// vcc = black, gnd = green
SoftwareSerial nss(7,6);

// Coordinate to start from 
// 55.90477, 12.49889
static const float COORDINATE_LAT = 55.90477;
static const float COORDINATE_LON = 12.49889;

time_t codetime; // save now() in this - also used to seed the PRNG

//  tællere til nedtælling - vi starter på 30 minutter (1800 sekunder)
// formel: currentIntercal = defaultInterval*(distance(m)+1)*(hastighed(km/t)+1) 
// skal trækkes fra countdown-timer (remainCount) hvert sekund
float startCount = 30;
float remainCount = startCount;

float defaultInterval = 0.01;


long previousMillis;
int interval = 1000; // refresh interval for the display

void setup() {
  pinMode(txpowerPin,OUTPUT); // setting the power-pin for the 433 .. low to start with
  digitalWrite(txpowerPin,HIGH); // .. meaning, HIGH while we debug 

  // configure virtualwire
  vw_set_tx_pin(RF_TX_PIN); // Setup transmit pin
  vw_setup(2000); // Transmission speed in bits per second.
  
  Serial.begin(9600);
  Serial.println("please wait..");
  nss.begin(9600); // initiate SoftwareSerial, which we use to talk to the GPS

  lcd.begin(16,2);               // initialize the lcd 
  lcd.setBacklight(HIGH); // LOW  = slukket backlight, HIGH = .. guess
  // for demo, we just send out the code
 
  sendCode("AX4321");
}

void loop() {
  // Pseudocode for later
  // set and check states for the loop
  // Different phases: startup (wait for signal), Aquiring code (getting the user to move), code aquired (send the user back), back to the box (send code to box)
  // I should use the game-state/tasknr as I did in TheBox - but this time there is no saved states, so tasknr should be sufficient
  
  // I should allow for the GPS-signal to get lost after the initial tasknr, so the countdown doesn't get reset
  // to be safe, end() the gps-module before talking to 433
  // end of pseudocode for now
  
    // stuff the gps-object with data
    feedgps();
    gps.f_get_position(&flat, &flon, &age);
    gps.crack_datetime(&Year, &Month, &Day, &Hour, &Minute, &Second, NULL, &age); // this will probably fail at first..

     // this might not be needed as we need to talk to the GPS 'all the time' even after first fix
    //updatedatetime(); // runs for 1000ms to make sure we have date-time correct

    if (age < 1000) {
      // I only want to update the display once per interval
      int speed = (int)gps.f_speed_kmph(); // a sort of rounding
      if (millis()-previousMillis >= interval) {
        if (displayState == 2) { displayState = 0; } 
        lcd.setCursor(0,0); 
        lcd.print("                ");
        lcd.setCursor(0,0);
        if (speed < 100) { 
            lcd.print(" "); 
        }
        if (speed < 10) { 
            lcd.print(" "); 
        }
        lcd.print(speed);

        // here we have a rotating 'field' that toggle between hdop and age
        lcd.print(" / "); 
        
        switch(displayState) {
          case 0:
            lcd.print(" hdop: ");lcd.print(gps.hdop());
          break;
          case 1:
            lcd.print(" age:");lcd.print(age);         
          break;         
        }

        lcd.setCursor(0,1);
        lcd.print(Year); 
        lcd.print("-");
        if (Month < 10) { lcd.print("0"); } lcd.print(Month);
        lcd.print("-");
        if (Day < 10) { lcd.print("0");} lcd.print(Day);
        lcd.print(" ");
        if (Hour < 10) { lcd.print("0"); } lcd.print(Hour);
        lcd.print(":");
        if (Minute < 10) { lcd.print("0"); } lcd.print(Minute);
        previousMillis = millis();
        displayState++;
        const char *msg = "updating display";
        // change to generated code - remember to prepend safety char
  //      sendCode("AX4321");



    } // eo 'display'

  } else { 
    if (millis()-previousMillis >= interval) {
      lcd.clear(); lcd.print("Please Wait.."); 
//      lcd.clear(); lcd.print("Updating safe.."); 
      previousMillis = millis();
      //const char *msg = "Waiting for signal";
      // for demo, we send out our code every second
//      sendCode("AX4321");
    }
  } 
}


static bool feedgps()
{
  while (nss.available())
  {
    if (gps.encode(nss.read()))
      return true;
  }
  return false;
}


void updatedatetime() {
         start = millis();
         while (millis() - start < 1000) {
            feedgps();
         }
}

// function for sending the code to the motherCache - I am in doubt if the syntax is correct, but time will tell
void sendCode(char *m) {
//  const char *msg = "hello";
  vw_send((uint8_t *)m, strlen(m));
}
