# About Overlook Horizon Inc.

Overlook Horizon Inc. (**OLHZN**) is a 501(c)(3) non-profit charitable organization in the United States with the goal of educating and inspiring the next generation of scientists and engineers to shape our world. We fly high altitude weather balloons for science and fun! If you like our work, please consider making a tax-deductible donation to help support us at: http://donorbox.org/olhzn

###### Follow Us Online
- **Website:** http://overlookhorizon.com
- **Facebook:** http://facebook.com/overlookhorizon
- **Twitter:** http://twitter.com/OLHZN
- **Instagram:** http://instagram.com/OLHZN
- **YouTube:** http://youtube.com/OLHZN
- **Discord:** https://discord.gg/vxnAf4T
- **Donate:** http://donorbox.org/OLHZN

## About the OLHZN-Track Software

This is originally a fork of David Akerman's FlexTrack library: https://github.com/daveake/FlexTrack

This is an Arduino based radio position tracking & data logging system intended for high altitude weather balloons flights.

This code provides transmission of radio telemetry via APRS as well as telemetry logging on OLHZN Arduino Mega compatible circuit boards for High Altitude Ballooning.

#### Software Dependencies ####

This software requires some Arduino libraries in order to function.  Most of them can be installed in the Arduino IDE by going to the menu options **Sketch** >> **Include Library** >> **Manage Libraries...** and then search for the library name **OR** by clicking the links below to manually download the library ZIP file then manually add it to your Arduino sketch from **Sketch** >> **Include Library** >> **Add .ZIP Library**

- [RTClib](https://github.com/adafruit/RTClib)
- [NewTone](https://bitbucket.org/teckel12/arduino-new-tone/wiki/Home)
- [OneWire](https://github.com/bigjosh/OneWireNoResistor/)
- [DallasTemperature](https://github.com/milesburton/Arduino-Temperature-Control-Library)
- [TinyGPS++](https://github.com/mikalhart/TinyGPSPlus)
- [DHT](https://github.com/adafruit/DHT-sensor-library)
- [SFE_BMP180](https://github.com/sparkfun/BMP180_Breakout/tree/master/Libraries/Arduino)
- [SD](https://github.com/adafruit/SD)
- [Adafruit_Sensor](https://github.com/adafruit/Adafruit_Sensor)
- [Adafruit_BME280](https://github.com/adafruit/Adafruit_BME280_Library)

## Buy an OLHZN Circuit Board

This code is designed for our OLHZN Arduino Mega compatible circuit board (Revision 5). You're certainly welcome to use and adapt this code for your own projects, but you can also purchase one of our circuit boards here: http://overlookhorizon.com/buy-circuit-boards

A parts list and assembly instructions for our OLHZN Circuit Boards can be found here: [/OLHZN_Track/Documentation/](https://github.com/OverlookHorizon/OLHZN_Track/tree/master/Documentation)


## How to launch a high altitude weather balloon

Need further guidance on cameras, balloons, parachutes, etc.? See our how-to guide here: http://overlookhorizon.com/how-to

## Disclaimer

The OLHZN_Track code is provided as is with no guarantees of performance or operation. 

If you decide to use this code under a balloon it is your responsibility to ensure you comply with the local legislation and laws regarding unmanned free meteorological balloon launching and airborne radio transmission.

The Radiometrix HX-1 144.390 MHz is **NOT** license exempt in the United States of America and **requires** an amateur radio license issued by the FCC.

Use of APRS requires an amateur radio license in many countries and a number of countries don’t permit the airborne use of APRS under any circumstances. 

It is YOUR responsibility to ensure your hardware and code is used safely and legally.

## Further Reading on High Altitude Weather Balloon Safety

Please read this http://overlookhorizon.com/safety

## License

The hardware design & code is released under a Creative Commons License 3.0 Attribution-ShareAlike License : http://creativecommons.org/licenses/by-sa/3.0/
