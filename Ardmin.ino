// Made by Lucas Oberwager, 10/5/2020
#include <Wire.h>
#include <NeoSWSerial.h>
#include <NMEAGPS.h>
#include <GPSport.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

#define I2C_ADDRESS 0x3C
SSD1306AsciiWire oled;
// Use faster I2C library, if theres no memory left change this back.
#define OPTIMIZE_I2C 1

static NMEAGPS  gps; // This parses received characters
static gps_fix  fix; 
NeoSWSerial ss(4, 3); // The serial connection to the GPS device
    
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
// Max Speed
int mspd = 0;
// Avg speed
float spdSum=0;
int spdSamples=1; // Fix divide by zero error
// Boolean to see if the ride is paused
bool paused = false;
// Boolean to see if the ride changed its paused state
bool deltaPaused;

unsigned long currentMillis;
unsigned long pausedMillis = 0;
unsigned long pauseTime;

void setup() {
  Wire.begin();
  Wire.setClock(400000L);
  
  //Serial.begin(9600);
  gpsPort.begin(9600);

  oled.begin(&Adafruit128x64, I2C_ADDRESS);

  oled.setFont(System5x7);
  oled.clear();
  oled.set2X();
  //Serial.println("Starting...");
  oled.print("Ardmin v2");
  oled.println();
  oled.set1X();
  oled.print("By Lucas Oberwager");
  oled.println();
  oled.print("Booting up..."); 
  oled.println();
  oled.print("Searching for GPS..."); 
  oled.set2X();
}

void loop() {
  while (gps.available( gpsPort )) {
    fix = gps.read();
    currentMillis = millis();
    if (!paused) {
      // Go to 0,0 on the oled to override previous frame without clearing to save time
      oled.home();


      // Update current lat and lng
      if (fix.valid.location) {
        // Update current lat,lng
        clat = fix.latitude();
        clng = fix.latitude();
      } else {
        // Dont start ride until lat & lng fix is found
        return;
      }


      // Code to run only on first loop
      if (plat == 0) {
        oled.clear();
        plat = clat;
        plng = clng;
        // Print total distance travelled, which at start is just 0
        oled.print(0.00);
      }

      
      // Update total distance
      if (currentMillis - previousMillis >= 10000 || plat == 0) {
        // Code to run every 10 seconds (10000 ms) or on first run

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
      if (fix.valid.speed) {
        cspd = (int) round(fix.speed_mph());

        if (cspd <= 1) {
          // We are not moving/moving slowly; Pause!
          paused = true;
          deltaPaused = true;
        }
        
        if (cspd > mspd) {
          // Set the new max speed
          mspd = cspd;
        }

        // Avg speed calculator
        spdSum += cspd;
        spdSamples++;

        // Set column to right align
        // If cspd is zero, manually set length to 1, otherwise just take the log of cspd + 1
        // 36 is pixel length of string "MPH"
        oled.setCol(oledDisplayWidth-(oled.fieldWidth(cspd==0 ? 1 : (floor(log10(cspd)) + 1) ) + 36)  );
        oled.print(String(cspd) + "MPH");
      }
      
      
      // Print elapsed ride time
      
      // Center align and center horizontal
      // 84 = oled.fieldWidth(7) // 7 = number of characters in time string
      oled.setCursor((oledDisplayWidth-84)/2, 3);
      // Convert uptime (ride time) to hours minutes seconds
      oled.print(millistoHMS(millis()/1000));


      // Print current time
      
      // Center align and at last row
      // 60 = oled.fieldWidth(5) // 5 = number of characters in time string
      oled.setCursor((oledDisplayWidth-60)/2, 6);
      static char str[5];
      sprintf(str, "%02d:%02d", fix.dateTime.hours % 12 == 7 ? 12 : (fix.dateTime.hours + 5) % 12, fix.dateTime.minutes);
      oled.print(str);
    } else {
      // Ride is paused
      
      if (deltaPaused) {
        // The ride paused this loop

        // Record time when paused
        pauseTime = millis();

        oled.clear();
        // Alert the ride was paused
        // 132 = oled.fieldWidth(11) // 11 = number of characters in string
        oled.print("RIDE PAUSED");
        oled.println();
        
        // Set smaller font size to summarize
        oled.set1X();
        
        // Print max speed
        oled.print("Max Spd: " + String(mspd) + "MPH");
        oled.println();
        
        // Print Avg speed
        oled.print("Avg Spd: " + String(spdSum / spdSamples) + "MPH");
        oled.println();

        // Print total distance
        oled.print("Distance: " + String(gpsDistance, 2) + "mi");
        oled.println();

        // Print Current time
        static char str[20];
        sprintf(str, "Current Time: %02d:%02d", fix.dateTime.hours % 12 == 7 ? 12 : (fix.dateTime.hours + 5) % 12, fix.dateTime.minutes);
        oled.print(str);
        oled.println();

        // Print Total time
        oled.print("Elapsed time: " + String(millistoHMS(currentMillis/1000)));
        oled.println();

        // Print moving time
        oled.print("Riding time: " + String(millistoHMS((currentMillis-pausedMillis)/1000)));
        oled.println();

        deltaPaused = false;
      } else {
        // This ride has been paused for a while...

        // Measure speed to see if we are moving
        cspd = (int) round(fix.speed_mph());
        Serial.println(fix.speed_mph());
        if (cspd > 1) {
          // We are moving!
          deltaPaused = true;
          paused = false;
          // compare current time to when ride was paused
          pausedMillis += currentMillis - pauseTime;
          // reset font size
          oled.set2X();
          // clear the oled screen
          oled.clear(); 
        } else {
          // Not moving, update times
          oled.setRow(5);
          // Print Current time
          static char str[20];
          sprintf(str, "Current Time: %02d:%02d", fix.dateTime.hours % 12 == 7 ? 12 : (fix.dateTime.hours + 5) % 12, fix.dateTime.minutes);
          oled.print(str);
          oled.println();

          // Print Total time
          oled.print("Elapsed time: " + String(millistoHMS(millis()/1000)));
          oled.println();
        }
      }
    }
  }
}

float calc_dist(float flat1, float flon1, float flat2, float flon2)
{
  // Calculate distance from starting point using Haversine formula
  //formula:  a = sin²(Δφ/2) + cos φ1 ⋅ cos φ2 ⋅ sin²(Δλ/2)
  //c = 2 ⋅ atan2( √a, √(1−a) )
  //d = R ⋅ c
  //where φ is latitude, λ is longitude, R is earth’s radius (mean radius = 6,371km);
  float dist=0;
  float dist2=0;
  float diflat=0;
  float diflon=0;
  
  // I've to split all the calculation in several steps. If i try to do it in a single line the arduino will explode.
  diflat=radians(flat2-flat1);
  flat1=radians(flat1);
  flat2=radians(flat2);
  diflon=radians((flon2)-(flon1));
  
  dist = (sin(diflat/2.0)*sin(diflat/2.0));
  dist2= cos(flat1);
  dist2*=cos(flat2);
  dist2*=sin(diflon/2.0);
  dist2*=sin(diflon/2.0);
  dist +=dist2;
  
  dist=(2*atan2(sqrt(dist),sqrt(1.0-dist)));

  // 3958.76571 is earths circumference in meters divided by 1609 to get miles
  dist*=3958.76571; //Converting to miles
  
  //Serial.println(dist_calc);
  return dist;
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
