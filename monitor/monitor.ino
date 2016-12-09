
/*
   Washer & Dryer Status Monitor
   Using ESP8266 adafruit feather huzzah
   Push notifications via PushOver API

   by Christopher Brasington 2016

   https://github.com/chrisbrasington/washer-dryer-pushover-status-monitor-ESP8266
*/
#include <SPI.h>
#include <ESP8266WiFi.h>

// include wifi username/password and pushover api keys
// see sample file for example
#include "keys.h"

// wifi server for sending messages to pushover
WiFiServer server(80);

// input pins for vibration sensors into arduino
int washerPin = 4;
int dryerPin = 5;

// counter for movement detection "shakes"
long washerShakes = 0;
long dryerShakes = 0;

// counter for no movement detection "rest"
int washerRest = 0;
int dryerRest = 0;

// delay read
// delay individual checks (100ms = 0.1s)
int delayRead = 100;

// reset interval in milliseconds
// 0.1s * 600 = 60 seconds = 1 minute
// if resting for 1 minute, detect a state change (from ON to OFF)
//    or reset counters to remove noise
int resetIntervalWasher = 1500; // 2.5 minutes (rinse mid-cycle is longer with no shakes)
int resetIntervalDryer = 1200;   // 2 minutes

// how much movement must be detected before resetInterval 
//   to be considered active?
int threshold = 300;

// initial setup function run immediately on power-on of ESP8266/arduino
void setup() {
  // begin serial output
  Serial.begin(115200);
  delay(100);

  // setup pins
  pinMode(washerPin, INPUT);
  pinMode(dryerPin, INPUT);

  // connect to wifi
  connectWifi();

  server.begin();
}

// main loop
void loop() {

  // detect washer shake
  if(digitalRead(washerPin) == 1) {
    washerShakes = washerShakes + 1;
    washerRest = 0;
  }
  // detect washer rest
  else {
    washerRest = washerRest + 1;
  }

  // detect dryer shake
  if(digitalRead(dryerPin) == 1) {
    dryerShakes = dryerShakes + 1; 
    dryerRest = 0;
  }
  // detect dryer rest
  else {
    dryerRest = dryerRest + 1;
  }

  // if no washer movement for 1 minute
  if(washerRest > resetIntervalWasher) {
    // if washer has been active enough
    if(washerShakes > threshold) {
      // detect washer state switch from ON to OFF
      pushOver("Washer has finished running.");
    }
    washerShakes = 0;
    washerRest = 0;
  }
  bool bothRunning = false;
  // if no dryer movement for 1 minute
  if(dryerRest > resetIntervalDryer) {
    // if dryer has been active enough
    if(dryerShakes > threshold) {
      // detect dryer state switch from ON to OFF
      pushOver("Dryer has finished running.");
    }
    dryerShakes = 0;
    dryerRest = 0;
  }
  // allow a running dryer to suppress the washer
  // since dryer runs much longer and a load cannot yet 
  //   be moved from the washer to the dryer,
  //   this allows only the dryer to be the notifier
  //   of the completion of both machines
  else if(dryerShakes > threshold) {
    washerShakes = 0;
    washerRest = 0;
    bothRunning = true;
  }

  Serial.print("Washer Shakes (");
  Serial.print(washerShakes);
  Serial.print(") Rest (");
  Serial.print(washerRest);
  Serial.print(")");
  if(bothRunning) {
    Serial.print(" (suppressed)");
  }
  Serial.print(" --- Dryer Shakes (");
  Serial.print(dryerShakes);
    Serial.print(") Rest (");
  Serial.print(dryerRest);
  Serial.println(")");
 
  // delay individual checks (100ms = 0.1s)
  delay(delayRead);       
}

// connect to wifi
void connectWifi() {
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  Serial.println();
  pushOver("Washer/Dryer Monitor is online.");
}

void printLine(){
  Serial.print("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
}

/*
   Pushover function by M.J. Meijer 2014
   Send pushover.net messages from the arduino
*/
byte pushOver(char *pushovermessage)
{
 String message = pushovermessage;
 printLine();
 Serial.print("Sending message to pushover: ");
 Serial.println(message);
 printLine(); 

 WiFiClient client = server.available();   // listen for incoming clients

 int length = 81 + message.length();

 if(client.connect(pushoversite,80))
 {
   client.println("POST /1/messages.json HTTP/1.1");
   client.println("Host: api.pushover.net");
   client.println("Connection: close\r\nContent-Type: application/x-www-form-urlencoded");
   client.print("Content-Length: ");
   client.print(length);
   client.println("\r\n");;
   client.print("token=");
   client.print(apitoken);
   client.print("&user=");
   client.print(userkey);
   client.print("&message=");
   client.print(message);
   while(client.connected())  
   {
     while(client.available())
     {
       char ch = client.read();
       Serial.write(ch);
     }
   }
   client.stop();
 }  
}
