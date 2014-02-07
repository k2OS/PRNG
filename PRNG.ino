#include <LiquidCrystal_I2C.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <Time.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x20,4,5,6,0,1,2,3,7,NEGATIVE); 

/*
#include <MemoryFree.h>
#include <pgmStrToRAM.h>
*/
TinyGPSPlus gps;
// rx = 6 (yellow), tx = 7 (white)
// vcc = black, gnd = green
SoftwareSerial nss(7,6);

// Coordinate to start from 
static const float COORDINATE_LAT = 55;
static const float COORDINATE_LON = 12;

time_t rightnow; // save now() in this - also used to seed the PRNG

//  tællere til nedtælling - vi starter på 30 minutter (1800 sekunder)
// formel: currentInterval = defaultInterval*distance(m)*hastighed(km/t) 
// skal trækkes fra countdown-timer hvert sekund
float defaultInterval = 0.01;


long previousMillis;
int interval = 1000; // refresh interval for the display

void setup() {
  //Serial.begin(9600);
  nss.begin(9600); // initiate SoftwareSerial, which we use to talk to the GPS

  lcd.begin(16,2);               // initialize the lcd 
  lcd.setBacklight(HIGH); // LOW  = slukket backlight, HIGH = .. guess
}

void loop() {
    // stuff the gps-object with data
    feedgps();

    if (gps.location.isValid()) {
      // I only want to update the display once per interval
      int speed = (int)gps.speed.kmph(); // a sort of rounding
      if (millis()-previousMillis >= interval) {
        lcd.setCursor(0,0);
        lcd.print("                ");
        lcd.setCursor(0,0);
        lcd.print(gps.location.lat(), 6);

        lcd.setCursor(0,1);
        lcd.print("                ");
        lcd.setCursor(0,1);
        lcd.println(gps.location.lng(), 6);

        lcd.setCursor(10,0); lcd.print("     ");
        lcd.setCursor(10,0); 
        if (speed > 9) { 
          lcd.print("  "); 
        }  else if (speed > 99) { lcd.print(" "); } 

        lcd.print(speed);
    //    Serial.print("LAT="); Serial.println(gps.location.lat(), 6);
    //    Serial.print("LNG="); Serial.println(gps.location.lng(), 6);

        previousMillis = millis();
        // Set the internal time to time received from the GPS - if time is valid
        if (gps.time.isValid()) {
          // settime(hour, minute, second, day, month, year)
          setTime(gps.time.hour(), gps.time.minute(), gps.time.second(), gps.date.day(), gps.date.month(), gps.date.year()); 
          rightnow = now(); // setting the "Unix"-time - I will evaluate if I want to create a code later
        }
      }
        /*
      randomSeed(i); // "now"
      randNumber = random(999,9999);
      Serial.println(randNumber);
      codes[index] = randNumber;
      lcd.print("Code: "); lcd.print(codes[0]);  
*/

  } else { 
    if (millis()-previousMillis >= interval*2) {
      lcd.clear(); lcd.print("No fix.."); 
      Serial.print("No fix.."); Serial.println(gps.location.age());
      previousMillis = millis();
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

