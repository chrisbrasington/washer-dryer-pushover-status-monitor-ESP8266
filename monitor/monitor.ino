#include <SPI.h>
#include <ESP8266WiFi.h>

// include wifi username/password and pushover api keys
// see sample file for example
#include "keys.h"

// wifi server for sending messages to pushover
WiFiServer server(80);

bool washerRunning = false;
bool dryerRunning = false;

int washerPin = 4;
int dryerPin = 5;

int washerVal = 0;
int dryerVal = 0;


int loopDelay = 60000; // five minutes = 300000, 1 minute = 60000
int checkDelay = 1000;  // 1,000 ms = 1 seconds

// how many seconds out of polling (minute) must movement be felt
//   to be considered "running"
int frequencyAllowance = 2;

void setup() {
  // begin serial output
  Serial.begin(115200);
  delay(100);

  pinMode(washerPin, INPUT);
  pinMode(dryerPin, INPUT);

  // connect to wifi
  connectWifi();

  server.begin();
}

void loop() {
  setStatus();

  delay(loopDelay);
}

void setStatus() {

  Serial.print("\nReading Status");

  bool priorWasherStatus = washerRunning;
  bool priorDryerStatus = dryerRunning;
  
  int leftWasher=0;
  int rightWasher=0;
  int leftDyer=0;
  int rightDryer=0;

  // poll for an entire minute
  for(int i=0;i<=59;i++) {
    washerVal = digitalRead(washerPin);
    dryerVal = digitalRead(dryerPin);
    if(washerVal == 0) {
      leftWasher = leftWasher+1;
    }
    else {
      rightWasher = rightWasher+1;
    }
    if(dryerVal == 0) {
      leftDyer = leftDyer+1;
    }
    else {
      rightDryer = rightDryer+1;
    }

    if(i%10==0) {
      Serial.print(".");
    }
    delay(checkDelay);       
    
  }

  if(leftWasher == 0 || rightWasher == 0){
    // no washer movement
    washerRunning = false;
  }
  else {
    // movement
    if(leftWasher > frequencyAllowance || rightWasher > frequencyAllowance){
      washerRunning = true;
    }
  }

  if(leftDyer == 0 || rightDryer == 0){
    // no washer movement
    dryerRunning = false;
  }
  else {
    // movement
    if(leftDyer > frequencyAllowance || rightDryer > frequencyAllowance){
      dryerRunning = true;
    }
  }

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
 WiFiClient client = server.available();   // listen for incoming clients
 String message = pushovermessage;

 Serial.print("Sending message to pushover: ");
 Serial.println(message);
 Serial.println();

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
