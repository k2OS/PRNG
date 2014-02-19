#include <LiquidCrystal_I2C.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>
// #include <EEPROM.h>
#include <Time.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x20,4,5,6,0,1,2,3,7,NEGATIVE); 

/*
#include <MemoryFree.h>
#include <pgmStrToRAM.h>
*/

/* GPS-related variables */
float flat, flon;
unsigned long age, date, time;
int Year;
byte Month, Day, Hour, Minute, Second;

long int start; // timer for letting the gps-object 'feed' more than once

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
  Serial.begin(9600);
  Serial.println("please wait..");
  nss.begin(9600); // initiate SoftwareSerial, which we use to talk to the GPS

  lcd.begin(16,2);               // initialize the lcd 
  lcd.setBacklight(HIGH); // LOW  = slukket backlight, HIGH = .. guess
}

void loop() {
    // stuff the gps-object with data
    feedgps();
    gps.f_get_position(&flat, &flon, &age);
    gps.crack_datetime(&Year, &Month, &Day, &Hour, &Minute, &Second, NULL, &age); // this will probably fail at first..54

     // this might not be needed as we need to talk to the GPS 'all the time' even after first fix
    //updatedatetime(); // runs for 1000ms to make sure we have date-time correct

    if (age < 1000) {
      // I only want to update the display once per interval
      int speed = (int)gps.f_speed_kmph(); // a sort of rounding
      if (millis()-previousMillis >= interval) {
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

        lcd.print(" / "); lcd.print(gps.hdop());

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
    }

  } else { 
    if (millis()-previousMillis >= interval) {
      lcd.clear(); lcd.print("Please Wait.."); 
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


void updatedatetime() {
         start = millis();
         while (millis() - start < 1000) {
            feedgps();
         }
}
