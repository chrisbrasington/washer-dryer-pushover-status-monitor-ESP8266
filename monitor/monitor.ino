
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

// booleans which hold if each machine is running
bool washerRunning = false;
bool dryerRunning = false;

// input pins for vibration sensors into arduino
int washerPin = 4;
int dryerPin = 5;

// values read from vibration sensor input pins
int washerVal = 0;
int dryerVal = 0;

// delay interval of main loop
int loopDelay = 60000; // five minutes = 300000, 1 minute = 60000

// delay interval of checking (every 2 seconds)
int checkDelay = 2000;  // 1,000 ms = 1 seconds

// amount of times to check
// 118 (2 minutes) of checks, but checking every 2 seconds
//   is actually 4 minutes of polling interval total
int pollingFrequency = 118;

// how many seconds out of polling (minute) must movement be felt
//   to be considered "running"
// this is useful to allow a % margin of error for shakes when not running
int frequencyAllowance = 12;

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

void loop() {
  checkStatus();

  delay(loopDelay);
}

void checkStatus() {

  Serial.print("\nReading Status");

  // cache prior status (if changed, notify)
  bool priorWasherStatus = washerRunning;
  bool priorDryerStatus = dryerRunning;

  // using a digital (not analog read)
  // counters for the number of left/right hits
  //   on the 2 vibration sensors
  int leftWasher=0;
  int rightWasher=0;
  int leftDryer=0;
  int rightDryer=0;

  // poll pollingFrequency # of times
  for(int i=0;i<=pollingFrequency;i++) {

    // read both pins
    washerVal = digitalRead(washerPin);
    dryerVal = digitalRead(dryerPin);

    // increment counters
    if(washerVal == 0) {
      leftWasher = leftWasher+1;
    }
    else {
      rightWasher = rightWasher+1;
    }
    if(dryerVal == 0) {
      leftDryer = leftDryer+1;
    }
    else {
      rightDryer = rightDryer+1;
    }
    if(i%10==0) {
      Serial.print(".");
    }

    // delay individual checks (every checkDelay seconds)
    delay(checkDelay);       
  }

  // detect no movement on washer
  if(leftWasher == 0 || rightWasher == 0){
    washerRunning = false;
  }
  // detect "allowance" movement on washer
  else {
    // movement
    if(leftWasher > frequencyAllowance || rightWasher > frequencyAllowance){
      washerRunning = true;
    }
  }

  // detect no movement on dryer
  if(leftDryer == 0 || rightDryer == 0){
    dryerRunning = false;
  }
  // detect "allowance" movement on dryer
  else {
    // movement
    if(leftDryer > frequencyAllowance || rightDryer > frequencyAllowance){
      dryerRunning = true;
    }
  }

  // print status 
  Serial.print("\nWasher status: ");
  Serial.println(washerRunning);
  Serial.print("Dryer  status: ");
  Serial.print(dryerRunning);
  Serial.println();

  // if washer has stopped running (and dryer isn't running)
  if(priorWasherStatus && !washerRunning && !dryerRunning) {
    // notify
    Serial.println("Washer is done!");
    pushOver("Washer is done running");
  }
  // if dryer has stopped running
  if(priorDryerStatus && !dryerRunning)
  {
    // notify
    Serial.println("Dryer is done!");
    pushOver("Dryer is done running");
  }
}

// connect to wifi
void connectWifi() {
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
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

  pushOver("Washer/Dryer Monitor is online.");
}

/*
   Pushover function by M.J. Meijer 2014
   Send pushover.net messages from the arduino
*/
byte pushOver(char *pushovermessage)
{
 String message = pushovermessage;
 Serial.print("Sending message to pushover: ");
 Serial.println(message);
 Serial.println();
 
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
