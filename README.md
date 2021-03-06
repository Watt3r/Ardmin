# Ardmin
## Simple Arduino bike computer, created by Lucas Oberwager

Ardmin is a cheap, lightweight GPS device for bikes.  

### Functions

This project is made to replicate other bike computers. There is a GPS speedometer, tracking average speed of a ride, as well as the top speed. The GPS module also tracks the ride distance. When no motion is detected for a configurable amount of time, the Ardmin automatically pauses the ride, and displays a more detailed summary of the current ride statistics. 

### Hardware
* [NEO 6M GPS module](https://www.aliexpress.com/item/32833328722.html?spm=a2g0o.productlist.0.0.2156112brY3Cf2&algo_pvid=583b0c28-8f0a-4b80-8d64-28abf9a8e9f3&algo_expid=583b0c28-8f0a-4b80-8d64-28abf9a8e9f3-20&btsid=0ab6fb8315891706405772271e26d4&ws_ab_test=searchweb0_0,searchweb201602_,searchweb201603_)
* [Adafruit PowerBoost 500 Charger](https://www.adafruit.com/product/1944)
* [3.3v Voltage regulator](https://www.aliexpress.com/item/32732025305.html?spm=a2g0s.9042311.0.0.27424c4d2JLv11)
* [0.96" I2C Yellow-Blue SSD1306 OLED](https://www.aliexpress.com/item/32828425736.html?spm=a2g0s.9042311.0.0.27424c4d2JLv11)
* [Pro Mini 3.3v ATMega 328p](https://www.aliexpress.com/item/32821902128.html?spm=a2g0s.9042311.0.0.30594c4df8xZIr)
* Toggle Switch
#### Wiring
![Wiring Diagram](https://github.com/Watt3r/Ardmin/raw/master/images/Wiring.fzz.png "Wiring.fzz")
### Case
As with all great projects, the casing is made from an old pizza box. Plans are underway to create a 3d printed case, however.
### Software
#### Installing Libraries
This project uses the genius [SSD1306 Text only library](https://github.com/greiman/SSD1306Ascii) to save RAM. 

In order to interact with the NEO 6M module, the [NeoGPS library](https://github.com/SlashDevin/NeoGPS) is used.  

When using an arduino mini, there are not enough hardware serial ports. To get arount this, use the [NeoSWSerial library](https://github.com/SlashDevin/NeoSWSerial)

All of these libraries must be installed and placed into your Arduino Libraries folder before the software can be flashed.  
NeoGPS requires quite a bit of setup to work, but it is definitely worth the ssetup.  
### Next steps
2. 3D model case
3. Add pictures and working demo video
5. More Garmin features
    * Calories burned [here](https://keisan.casio.com/exec/system/1350958587)
8. Power percentage

