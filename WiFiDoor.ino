#include <Streaming.h>
#include <WiFiEsp.h>
#include "secrets.h"
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros

// for WiFi communication
char ssid[] = SECRET_SSID;         // your network SSID (name)
char pass[] = SECRET_PASS;        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

// for ThingSpeak
unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
const char * myReadAPIKey = SECRET_READ_APIKEY;

WiFiEspServer server(80);
WiFiEspClient client;
RingBuffer buf(64);
char cmdStr[128];


int doorPin =7 ;



void setup()
{
  Serial.begin(9600);
  initWiFi();
  ThingSpeak.begin(client);
  pinMode(doorPin,INPUT);
}

void loop()
{
  if(digitalRead(doorPin) == HIGH){
    writeDataToThingSpeak(1);
  }
  else if(digitalRead(doorPin) == LOW){
    writeDataToThingSpeak(0);
  }
  delay(16000);
}

// =========================

void initWiFi()
{
  Serial1.begin(9600);    // initialize serial for ESP module
  WiFi.init(&Serial1);    // initialize ESP module

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true); // don't continue
  }

  //Serial << "Attempting to start AP " << ssid << "\n";
  //status = WiFi.beginAP(ssid, 10, pass, ENC_TYPE_WPA2_PSK);
  Serial << "Attempting to connect AP " << ssid << "\n";
  status = WiFi.begin(ssid, pass);
  if(status == WL_CONNECT_FAILED)
  {
    Serial << "Failed to start AP.\n";
    while (true); // don't continue
  }

  Serial << "Access point started\n";
  printWifiStatus();
  server.begin();
  Serial << "Server started\n";
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial << "IP Address: " << ip << "\n";
}

void sendHttpResponse(WiFiEspClient client)
{
  client.print(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n"  // the connection will be closed after completion of the response
    "\r\n");
}

bool writeDataToThingSpeak(int doorState)
{
  // set the fields with the values
  ThingSpeak.setField(1, doorState);

  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
    return true;
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
    return false;
  }
}
