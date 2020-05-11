// Made by Lucas Oberwager, 10/5/2020
#include <Wire.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

#define I2C_ADDRESS 0x3C
SSD1306AsciiWire oled;
// Use faster I2C library, if theres no memory left change this back.
#define OPTIMIZE_I2C 1

TinyGPSPlus gps;  // The TinyGPS++ object
SoftwareSerial ss(4, 3); // The serial connection to the GPS device

const int oledDisplayWidth = 128;

// Current lat + lng
float clat , clng;
// Previous lat + lng
// Plat needs to be initialized to 0.0 to check if it has been set yet
// As long as plat is initialized it would be redundant to initialize plng
float plat = 0.0, plng;
// Time since distance tracker has been updated
unsigned long previousMillis = 0; 
// GPS distance tracker
// Updates every 10 seconds by counting distance from previous compared point to current location
float gpsDistance = 0;
// Current Speed
int cspd;

void setup() {
  Wire.begin();
  Wire.setClock(400000L);
  //Serial.begin(115200);
  ss.begin(9600);

  oled.begin(&Adafruit128x64, I2C_ADDRESS);

  oled.setFont(System5x7);
  oled.clear();
  oled.set2X();
  //Serial.println("Starting...");
  oled.print("Ardmin v1");
  oled.println();
  oled.set1X();
  oled.print("By Lucas Oberwager");
  oled.println();
  oled.print("Booting up..."); 
  oled.println();
  oled.print("> _");
  oled.set2X();
}

void loop() {
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
    {
      if (gps.location.isValid())
      {
        //Serial.println(oled.displayWidth());
        //Serial.println(oledDisplayWidth);
        // Update current lat,lng
        clat = gps.location.lat();
        clng = gps.location.lng();

        // Go to 0,0 on the oled to override previous frame without clearing to save time
        oled.home();

        // Code to run only on first loop
        if (plat == 0) {
          oled.clear();
          plat = clat;
          plng = clng;
          // Print total distance travelled, which at start is just 0
          oled.print(0.00);
        }
        
        // Update total distance
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= 10000) {
          // Code to run every 10 seconds (10000 ms)

          // Clear the top section of oled in case of unexpected bug
          // BUG: sometimes speed overrides distance traveled
          oled.clearToEOL();
          
          gpsDistance += calc_dist(clat, clng, plat, plng);
          
          // Save the last time coords were updated
          previousMillis = currentMillis;
          plat = clat;
          plng = clng;

          // Print total distance travelled
          oled.print(String(gpsDistance, 2));
        }

        // Print current speed
        cspd = round(gps.speed.mph());
        // Set column to right align
        // If cspd is zero, manually set length to 1, otherwise just take the log of cspd + 1
        // 36 is pixel length of string "MPH"
        oled.setCol(oledDisplayWidth-(oled.fieldWidth(cspd==0 ? 1 : (floor(log10(cspd)) + 1) ) + 36)  );
        oled.print(String(cspd) + "MPH");


        // Print elapsed ride time
        // Set column to middle of screen
        // 84 = oled.fieldWidth(7) // 7 = number of characters in time string
        oled.setCursor((oledDisplayWidth-84)/2, 3);
        // Convert uptime (ride time) to hours minutes seconds
        oled.print(millistoHMS(millis()/1000));

        // Print current time
        // Center align and at last row
        // 60 = oled.fieldWidth(5) // 5 = number of characters in time string
        oled.setCursor((oledDisplayWidth-60)/2, 6);
        static char str[5];
        sprintf(str, "%02d:%02d", gps.time.hour() % 12 == 7 ? 12 : (gps.time.hour() + 5) % 12, gps.time.minute());
        oled.print(str);
      }
    }
  delay(100);
}

float calc_dist(float flat1, float flon1, float flat2, float flon2)
{
  // Calculate distance from starting point using Haversine formula
  //formula:  a = sin²(Δφ/2) + cos φ1 ⋅ cos φ2 ⋅ sin²(Δλ/2)
  //c = 2 ⋅ atan2( √a, √(1−a) )
  //d = R ⋅ c
  //where φ is latitude, λ is longitude, R is earth’s radius (mean radius = 6,371km);
float dist_calc=0;
float dist_calc2=0;
float diflat=0;
float diflon=0;

//I've to spplit all the calculation in several steps. If i try to do it in a single line the arduino will explode.
diflat=radians(flat2-flat1);
flat1=radians(flat1);
flat2=radians(flat2);
diflon=radians((flon2)-(flon1));

dist_calc = (sin(diflat/2.0)*sin(diflat/2.0));
dist_calc2= cos(flat1);
dist_calc2*=cos(flat2);
dist_calc2*=sin(diflon/2.0);
dist_calc2*=sin(diflon/2.0);
dist_calc +=dist_calc2;

dist_calc=(2*atan2(sqrt(dist_calc),sqrt(1.0-dist_calc)));

// 3958.76571 is earths circumference in meters divided by 1609 to get miles
dist_calc*=3958.76571; //Converting to miles

//Serial.println(dist_calc);
return dist_calc;
}

char * millistoHMS(unsigned long t)
{
 static char str[12];
 long h = t / 3600;
 t = t % 3600;
 int m = t / 60;
 int s = t % 60;
 sprintf(str, "%01ld:%02d:%02d", h, m, s);
 return str;
}
