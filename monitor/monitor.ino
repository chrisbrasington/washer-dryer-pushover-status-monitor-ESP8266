
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

// poll for seconds
float secondRun = 59*5;

// amount of times to check
int pollingFrequency = 11600;

// how many seconds out of polling (minute) must movement be felt
//   to be considered "running"
// this is useful to allow a % margin of error for shakes when not running
int frequencyAllowance = 12;

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
  printLine();
  
  // check washer/dryer status and notify
  checkStatus();
}

// check status and notify
void checkStatus() {

  Serial.print("Reading Status");

  // cache prior status (if changed, notify)
  bool priorWasherStatus = washerRunning;
  bool priorDryerStatus = dryerRunning;

  float washerVibrationDetected = 0;
  float washerVibrationNotDetected = 0;
  float dryerVibrationDetected = 0;
  float dryerVibrationNotDetected = 0;



  Serial.print(" for ");
  Serial.print(secondRun/59);
  Serial.println(" minutes.");
  

  // using a digital (not analog read)
  // counters for the number of left/right hits
  //   on the 2 vibration sensors
  for(int second = 0; second < secondRun; second++) {
    float leftWasher=0;
    float rightWasher=0;
    float leftDryer=0;
    float rightDryer=0;

    // poll pollingFrequency # of times
    for(int ms=0;ms<=999;ms++) {
  
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
      // delay individual checks (every checkDelay seconds)
      delay(1);       
    }
    
    float percentageWasher = 0;
    if(leftWasher != 0 && rightWasher != 0) {
      if(leftWasher < rightWasher) {
        percentageWasher = (leftWasher/999)*100;
      }
      else {
        percentageWasher = (rightWasher/999)*100;
      }
    }
  
    if(percentageWasher > 10){
      washerVibrationDetected = washerVibrationDetected +1; 
    }
    else {
      washerVibrationNotDetected = washerVibrationNotDetected + 1;
    }

    float percentageDryer = 0;
    if(leftDryer != 0 && rightDryer != 0) {
      if(leftDryer < rightDryer) {
        percentageDryer = (leftDryer/999)*100;
      }
      else {
        percentageDryer = (rightDryer/999)*100;
      }
    }
  
    if(percentageDryer > 5){
      dryerVibrationDetected = dryerVibrationDetected +1; 
    }
    else {
      dryerVibrationNotDetected = dryerVibrationNotDetected + 1;
    }
  }
  Serial.println();
  Serial.print("Washer vibration: ");
  Serial.print((washerVibrationDetected/secondRun)*100);
  Serial.print("% running during ");
  Serial.print(secondRun/59);
  Serial.println(" minute duration.");
  
  washerRunning = (washerVibrationDetected/secondRun) > .10;

  if(washerRunning) {
    Serial.println("Washer is running.");
  }
  else {
    Serial.println("Washer is not running.");
  }

 // if washer has stopped running (and dryer isn't running)
  if(priorWasherStatus && !washerRunning && !dryerRunning) {
    // notify
    Serial.println("Washer is done!");
    pushOver("Washer is done running.");
  }
  else if(priorWasherStatus && !washerRunning && dryerRunning) {
    Serial.println("Washer is done, but dryer is running. Will not notify");
  }
  
  Serial.println();
  Serial.print("Dryer vibration: ");
  Serial.print((dryerVibrationDetected/secondRun)*100);
  Serial.print("% running during ");
  Serial.print(secondRun/59);
  Serial.println(" minute duration.");
  
  dryerRunning = (dryerVibrationDetected/secondRun) > .10;

  if(dryerRunning) {
    Serial.println("Dryer is running.");
  }
  else {
    Serial.println("Dryer is not running.");
  }

  // if dryer has stopped running
  if(priorDryerStatus && !dryerRunning)
  {
    // notify
    Serial.println("Dryer is done!");
    pushOver("Dryer is done running.");
  }
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
  Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
}

/*
   Pushover function by M.J. Meijer 2014
   Send pushover.net messages from the arduino
*/
byte pushOver(char *pushovermessage)
{
 String message = pushovermessage;
 Serial.print("Sending message to pushover: ");
 Serial.println(message);;

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
