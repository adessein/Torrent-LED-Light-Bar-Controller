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
 *  If errof while uploading, check that the serial termianl is closed
 *  
 *  # Requirements
 *  Arduino ESP8266 filesystem uploader
 *  https://github.com/esp8266/arduino-esp8266fs-plugin
 *  
 */
 
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>

String index_str_orig;

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

short C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16;
short prog;

void setup(){
  Serial.begin(115200);
  Serial.println("=== Torrent LED Bar ===");
  Serial.println("Starting ...");
  
  SPIFFS.begin();
  File f = SPIFFS.open("/TorrentBar.html", "r");
  while (f.available()){
    index_str_orig += char(f.read());
  }
  
  C1=C2=C3=C4=C5=C6=C7=0;

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
  webServer.on("/TorrentBar.html", HTTP_POST, handlePost);
  //webServer.on("/TorrentBar.html", HTTP_POST, dumpArgs);
  webServer.begin();
  Serial.println("OK !");
  writeRegister(0xFF,0xFF);
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}


void sendjQuery(){
  Serial.println("Sending jQuery... ");
  File f = SPIFFS.open("/jquery.min.js", "r");
  webServer.streamFile(f, "application/javascript");
  f.close();
}

void sendCSS(){
  Serial.println("Sending CSS... ");
  File f = SPIFFS.open("/styles.css", "r");
  webServer.streamFile(f, "text/css");
  f.close();
}


void sendIndex(){
  Serial.println("Sending index page... ");
  String index_str(index_str_orig);
  index_str.replace("{C1}",C1?"checked":"");
  index_str.replace("{C2}",C2?"checked":"");
  index_str.replace("{C3}",C3?"checked":"");
  index_str.replace("{C4}",C4?"checked":"");
  index_str.replace("{C5}",C5?"checked":"");
  index_str.replace("{C6}",C6?"checked":"");
  index_str.replace("{C7}",C7?"checked":"");
  index_str.replace("{C8}",C8?"checked":"");
  webServer.send(200, "text/html", index_str.c_str());
}
  
void writeRegister(byte bh, byte bl) {
  Serial.println("writeRegister");
  Serial.print(bh, BIN); Serial.print(" "); Serial.println(bl, BIN); 
  digitalWrite(loadPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, bh);
  shiftOut(dataPin, clockPin, MSBFIRST, bl);
  digitalWrite(loadPin, HIGH);
}

void dumpArgs(){
  for (int i=0; i<webServer.args(); i++){
    Serial.print(webServer.argName(i));
    Serial.print("=");
    Serial.println(webServer.arg(i));
  }
}


void handlePost(){

  Serial.println("handlePost");

  if (webServer.hasArg("user"))  {
    if (webServer.hasArg("mode"))  {
      
      Serial.print("Prog= "); Serial.println(prog);

      if (webServer.arg("mode") == "0")
      {
          Serial.println("Mode: flashing");
          C1=1;
      }
      if (webServer.arg("mode") == "1")
      {
          Serial.println("Mode: steady 1");
          C2=1;
      }
      if (webServer.arg("mode") == "2")
      {
          Serial.println("Mode: steady 2");
          C3=1;
      }
      if (webServer.arg("mode") == "3")
      {
          Serial.println("Mode: cruise");
          C4=1;
      }
      /*    
      if (prog == "Mode 1")           datah=0b01111111;
      if (prog == "Mode 2")           datah=0b10111111;
      if (prog == "Mode 3")           datah=0b11011111;
      if (prog == "Cruise Mode")      datah=0b11101111;
      if (prog == "Rear Left Arrow")  datah=0b11110111;
      if (prog == "Rear Right Arrow") datah=0b11111011;
      if (prog == "Take Downs")       datah=0b11111101;
      */
    }

    if(webServer.hasArg("name"))
    {
      if(webServer.arg("name") == "RightSide")     C8  = webServer.arg("value") == "true" ? 1:0;
      if(webServer.arg("name") == "LeftSide")      C9 = webServer.arg("value") == "true" ? 1:0;
      if(webServer.arg("name") == "Flashing")      C10 = webServer.arg("value") == "true" ? 1:0;
      if(webServer.arg("name") == "LowPower")      C11 = webServer.arg("value") == "true" ? 1:0;
      if(webServer.arg("name") ==  "FlashTD")      C12 = webServer.arg("value") == "true" ? 1:0;
      if(webServer.arg("name") ==  "DisableFront") C13 = webServer.arg("value") == "true" ? 1:0;
      if(webServer.arg("name") ==  "DisableRear")  C14 = webServer.arg("value") == "true" ? 1:0;
      if(webServer.arg("name") ==  "ProgramMode")  C15 = webServer.arg("value") == "true" ? 1:0;
    }
  
    datah = C1*128 + C2*64 + C3*32 + C4*16 + C5*8 + C6*4 + C7*2 + C8*1;
    datal = C9*128 + C10*64 + C11*32 + C12*16 + C13*8 + C14*4 + C15*2;
    datah = ~byte(datah);
    datal = ~byte(datal);
    writeRegister(datah, datal);
  }

}
