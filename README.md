# Washer & Dryer Status Monitor
### Push notifications via PushOver API

Setup:
- [Register a Pushover application](https://pushover.net/api)
- [Using ESP8266 with Arduino IDE](https://learn.adafruit.com/adafruit-feather-huzzah-esp8266/using-arduino-ide)

Hardware:
- [ESP8266 adafruit feather huzzah](https://www.adafruit.com/product/2821)
- 2x SW-420 Vibration Sensor Module Vibration Switch

Or
- [ESP8266 arduino](https://www.sparkfun.com/products/13678)
- FTDI usb 3.3v (for uploading code)
- 2x SW-420 Vibration Sensor Module Vibration Switch

### Wiring:
Vibration modules are simple to wire. Three pins: 3.3v, ground, and read. Change washerPin, dryerPin accordingly. 

### Configuration:
Rename keys_sample.h to keys.h and populate with wifi username/password, pushover apitoken and userkey.

### Run:
Serial output on all status/messagging. Notifications occur via [PushOver](https://pushover.net/).
Hardware will notify that it is online once during setup.
When washer finishes, if dryer is not running, washer will notify.
When dryer finishes, dryer will notify.

### Run duration:

You likely want to tweak the vibration sensitivity on the modules with a screwdriver.

You may want to alter the delay variables for washer/dryer cycles. My washer's rinse cycle isn't easily detected. I poll vibrations every second for 2 minutes to determine status. Then I delay looping for a minute. You may want to change this to 1 minute check and 5 minute delay, or whatever works for your machine.
