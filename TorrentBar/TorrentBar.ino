/*  Torrent LED Light Bar Controller
 *  The purpose of this project is to create a Wifi controller for an orange Axixtech Torrent Led Light Bar.  
 *  https://github.com/adessein/Torrent-LED-Light-Bar-Controller/
 *  GNU GENERAL PUBLIC LICENSE v3
 *  Arnaud DESSEIN
 */
 
/*  # Documentation
 *
 *  ## Captive portal
 *  https://github.com/ojack/ESP8266-captive-portal-webserver/blob/master/captivePortalFileServer.ino
 *  https://gist.github.com/bbx10/5a2885a700f30af75fc5
 *  https://gist.github.com/bbx10/667e3d4f5f2c0831d00b
 * 
 *  ## Javascript
 * 
 * 
 *  ## Filesystem
 *  https://steve.fi/Hardware/d1-flash/
 *  If error while uploading, check that the serial terminal is closed
 *  
 *  # Requirements
 *  Arduino ESP8266 filesystem uploader
 *  https://github.com/esp8266/arduino-esp8266fs-plugin
 * 
 *  # Arduino JSON
 *  https://arduinojson.org/v6/assistant/
 *  
 */

/* Example of requests
GET http://192.168.1.1/status
POST http://192.168.1.1/update?takeDownLights=true
*/
 
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include "ArduinoJson-v6.18.5.h"

String index_str;

const byte DNS_PORT = 53;
const char *ssid = "Torrent LED Bar";

const int dataPin = 12;   //Outputs the byte to transfer [D6]
const int loadPin = 15;   //Controls the internal transference of data in SN74HC595 internal registers (LATCH) [D8]
const int clockPin = 13; //Generates the clock signal to control the transference of data [D7]
byte datah = 0xFF;
byte datal = 0;

IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);


DynamicJsonDocument doc(1024);
bool crossPattern;
bool cruiseMode;
bool flashingTdsAlleys;
short lightheadPatterns[6]; // 0 to 19
short mode;
short flashPattern; // 0 to 20
short flashingTdsAlleysMode; // 0 to 3 (TD + Alley, Alley, TD)
short trafficArrowsMode; // 0 to 3 (left, right, center out)
short trafficArrowsPattern; // 0 to 10

//mod
bool rightSideAlley;
bool leftSideAlley;
bool lowPower;
bool frontCutoff;
bool rearCutoff;
bool takeDownLights;
//internals
bool W[16]; // state of the wires


void tapWire(int, int, bool);
void setup();
void loop();
void sendCSS();
void sendjQuery();
void writeRegister(byte, byte);
void dumpArgs();
void sendIndex();
void status();
void update();



void setup(){
  Serial.begin(115200);
  Serial.println("=== Torrent LED Bar ===");
  Serial.println("Starting ...");
  
  SPIFFS.begin();
  File f = SPIFFS.open("/TorrentBar.html", "r");
  while (f.available()){
    index_str += char(f.read());
  }
  
  // Initialisation of the state
  for(int i=0;i<16;i++) W[i] = false;
  for(int i=0;i<6;i++) lightheadPatterns[i] = 0;
  mode = 1;
  crossPattern = false;
  flashPattern = 0;
  flashingTdsAlleys = false;
  flashingTdsAlleysMode = 0;
  cruiseMode = false;
  trafficArrowsMode = 0;
  trafficArrowsMode = 0;
  trafficArrowsPattern = 0;
  // mods
  rightSideAlley = false;
  leftSideAlley = false;
  lowPower = false;
  frontCutoff = false;
  rearCutoff = false;
  takeDownLights = false;

  Serial.print("I/Os......... ");
  pinMode(dataPin, OUTPUT);
  pinMode(loadPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  Serial.println("OK !");
  
  Serial.print("Wifi......... ");
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid);
  Serial.println("OK !");

  Serial.print("DNS.......... ");
  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request  
  dnsServer.start(DNS_PORT, "*", apIP);
  Serial.println("OK !");

  Serial.print("Web server... ");
  webServer.on("/", sendIndex);
  webServer.on("/jquery.min.js", sendjQuery);
  webServer.on("/styles.css", sendCSS);
  webServer.on("/update", HTTP_POST, update);
  webServer.on("/status", HTTP_GET, status);

  webServer.begin();
  Serial.println("OK !");
  writeRegister(0xFF,0xFF);
}

void loop()
{
  dnsServer.processNextRequest();
  webServer.handleClient();
}


void sendjQuery()
{
  Serial.println("Sending jQuery... ");
  File f = SPIFFS.open("/jquery.min.js", "r");
  webServer.streamFile(f, "application/javascript");
  f.close();
}

void sendCSS()
{
  Serial.println("Sending CSS... ");
  File f = SPIFFS.open("/styles.css", "r");
  webServer.streamFile(f, "text/css");
  f.close();
}

  
void writeRegister(byte bh, byte bl)
{
  Serial.println("writeRegister");
  Serial.print(bh, BIN); Serial.print(" "); Serial.println(bl, BIN); 
  digitalWrite(loadPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, bl);
  shiftOut(dataPin, clockPin, MSBFIRST, bh);
  digitalWrite(loadPin, HIGH);
}

void dumpArgs()
{
  for (int i=0; i<webServer.args(); i++){
    Serial.print(webServer.argName(i));
    Serial.print("=");
    Serial.println(webServer.arg(i));
  }
}

void sendIndex()
{
  Serial.println("Sending index page... ");
  webServer.send(200, "text/html", index_str.c_str());
}

void status()
{
  Serial.println("status");
  // Returns the current state as JSON
  doc["mode"] = mode;

  doc["flashPattern"] = flashPattern; // Mode1 0 to 19
  doc["flashingTdsAlleysMode"] = flashingTdsAlleysMode; // 0 to 3 (TD + Alley, Alley, TD)
  doc["cruiseMode"] = cruiseMode;
  doc["trafficArrowsMode"] = trafficArrowsMode;
  doc["trafficArrowsPattern"] = trafficArrowsPattern;
  // mods
  doc["rightSideAlley"] = rightSideAlley;
  doc["leftSideAlley"] = leftSideAlley;
  doc["lowPower"] = lowPower;
  doc["frontCutoff"] = frontCutoff;
  doc["rearCutoff"] = rearCutoff;
  doc["takeDownLights"] = takeDownLights;

  // Light patterns (mode 2)
  JsonArray data = doc.createNestedArray("lp");
  for(int i=0;i<7;i++) data.add(lightheadPatterns[i]);

  char JSONmessageBuffer[1000];
  serializeJson(doc, JSONmessageBuffer, sizeof(JSONmessageBuffer));
  webServer.send(200, "json", JSONmessageBuffer);
}


void update()
{
  Serial.println("update");

  if (webServer.hasArg("mode"))
  {
    // {mode: int (1 to 3)}
    mode = webServer.arg("mode").toInt();
    
    if(mode==1)
    {
      W[0] = true;
      W[1] = false;
      W[2] = false;
      W[3] = crossPattern; 
    }
    if(mode==2)
    {
      W[0] = false;
      W[1] = true;
      W[2] = false;
    }
    if(mode==3)
    {
      W[0] = false;
      W[1] = false;
      W[2] = true;
    }
  }

  if(webServer.hasArg("cruiseMode")) cruiseMode = webServer.arg("cruiseMode") == "true" ? 1:0; // green-black

  if(webServer.hasArg("cruiseMoreLightheads"))
  {
    // Tap purple W[14] a single time
    tapWire(14, 1, false);
  }

  if(webServer.hasArg("flashPattern"))
  {
    flashPattern = webServer.arg("flashPattern").toInt();

    // Tap 3 times in a second W[9] to reset
    tapWire(9, 3, true);
    // Tap flashPattern times W[9]
    tapWire(9, flashPattern, false);
  }

  if(webServer.hasArg("lightheadPatterns"))
  {
    const size_t CAPACITY = JSON_ARRAY_SIZE(6);
    StaticJsonDocument<CAPACITY> doc;
    deserializeJson(doc,  webServer.arg("lp"));
    JsonArray array = doc.as<JsonArray>();

    for(int i=0;i<6;i++)
    {
      lightheadPatterns[i] = array[i].as<int>();
    }

    // Tap 3 times in a second W[14] to enter program mode
    tapWire(14, 3, true);
    
    // For each pair
    for(int i=0; i<6; i++)
    {
      // Tap 3 times in a second W[9] to reset
      tapWire(9, 3, true);
      // Tap lightheadPatterns[i] times W[9]
      tapWire(9, lightheadPatterns[i], false);
      // Tap W[14] to move to next pair
      tapWire(14, 1, false);
    }

    // Tap 3 times in a second W[14] to exit program mode
    tapWire(14, 3, true);
  }

  if (webServer.hasArg("trafficArrowsPattern"))
  {
    trafficArrowsPattern = webServer.arg("trafficArrowsPattern").toInt();

    // Tap 3 times in a second W[9] to reset 
    tapWire(9, 3, true);
    // Tap trafficArrowsPattern times W[9]
    tapWire(9, trafficArrowsPattern, false);
  }

  if(webServer.hasArg("flashingTdsAlleysMode"))
  {
    flashingTdsAlleysMode = webServer.arg("flashingTdsAlleysMode").toInt();
    
    flashingTdsAlleys = flashingTdsAlleysMode == 0 ? false:true;

    if(flashingTdsAlleysMode>0)
    {
      // I need to activate the wire now, since I need to configure
      setWire(11, true); // Red-black

      // Tap flashingTdsAlleysMode times W[9]
      tapWire(9, flashingTdsAlleysMode, false);
    }

  }

  if (webServer.hasArg("trafficArrowsMode"))
  {
    trafficArrowsMode = webServer.arg("trafficArrowsMode").toInt();
    W[4] = trafficArrowsMode == 1 || trafficArrowsMode == 3 ? 1:0; // orange
    W[5] = trafficArrowsMode == 2 || trafficArrowsMode == 3 ? 1:0; // blue
  }

  if (webServer.hasArg("mod"))
  {
    if(webServer.arg("mod") == "takeDownLights")    takeDownLights = webServer.arg("value") == "true" ? true:false; // brown-black
    if(webServer.arg("mod") == "rightSideAlley")    rightSideAlley  = webServer.arg("value") == "true" ? true:false; // orange-black
    if(webServer.arg("mod") == "leftSideAlley")     leftSideAlley = webServer.arg("value") == "true" ? true:false; // blue-black
    if(webServer.arg("mod") == "lowPower")          lowPower = webServer.arg("value") == "true" ? true:false;  // green 
    if(webServer.arg("mod") == "frontCutoff")       frontCutoff = webServer.arg("value") == "true" ? true:false; // yellow-black
    if(webServer.arg("mod") == "rearCutoff")        rearCutoff = webServer.arg("value") == "true" ? true:false; // green-black  
    
    if(webServer.arg("mod") == "crossPattern")    crossPattern = webServer.arg("value") == "true" ? true:false;  
    if(webServer.arg("mod") == "cruiseMode")      cruiseMode = webServer.arg("value") == "true" ? true:false;  
  }
  

  W[3] = cruiseMode | crossPattern; // gray cruise mode or cross pattern
  W[6] = takeDownLights; // brown-black
  W[7] = rightSideAlley; // orange-black
  W[8] = leftSideAlley; // blue-black
  W[9] = 0; //yellow is tapped
  W[10] = lowPower;  // green
  W[11] = flashingTdsAlleys; // red-black
  W[12] = frontCutoff; // yellow-black
  W[13] = rearCutoff; // green-black
  W[14] = 0; //purple is tapped

  setWires();
  delay(100);
  status();
}

void setWires()
{
  // Reset the bytes
  datal = (byte)0xFF;
  datah = (byte)0xFF;

  for (int i=0;i<8;i++)  datal = datal & ~(1 << W[i]); // Pull down
  for (int i=8;i<16;i++) datah = datah & ~(1 << W[i]); // Pull down

  writeRegister(datah, datal);
}

void setWire(int w, bool state)
{
  // state=true : Apply +12V
  // state=false : Apply 0V
  if(state==false) // I want to pull up
  {
    if(w<8)
    {
      datal = datal | (1 << w); // Pull up
    }
    else
    {
      datah = datah | (1 << w-8); // Pull up
    }
  }
  else
  {
    if(w<8)
    {
      datal = datal & ~(1 << w); // Pull down
    }
    else
    {
      datah = datah & ~(1 << w-8); // Pull down
    }
  }
  writeRegister(datah, datal);
}

void tapWire(int w, int N=1, bool fast=false)
{
  for(int i=0; i<N; i++)
  {
    setWire(w, true);
    delay(50);
    setWire(w, false);

    if(fast) // triple tap
    {
      delay(100);
    }
    else
    {
      delay(400); // 3 taps at 450ms is above 1 sec
    }
  }

}
