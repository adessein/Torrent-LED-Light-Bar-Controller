/*  Torrent LED Light Bar Controller
 *  The purpose of this project is to create a Wifi controller for an orange Axixtech Torrent Led Light Bar.  
 *  https://github.com/adessein/Torrent-LED-Light-Bar-Controller/
 *  GNU GENERAL PUBLIC LICENSE v3
 *  Arnaud DESSEIN
 */
 
/*  Documentation
 *  https://github.com/ojack/ESP8266-captive-portal-webserver/blob/master/captivePortalFileServer.ino
 *  https://gist.github.com/bbx10/5a2885a700f30af75fc5
 *  https://gist.github.com/bbx10/667e3d4f5f2c0831d00b
 *  https://onlinestringtools.com/escape-string
 */
 
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

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

short C1, C2, C3, C4, C5, C6, C7, C8;

const char index_page[] = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1" charset="utf-8" />
<style>
body {
  font-family: Sans-serif;
}

input[class="bttn"],p{
    font-size:24px;
}

.switch {
  position: relative;
  display: inline-block;
  width: 60px;
  height: 34px;
}

.switch input { 
  opacity: 0;
  width: 0;
  height: 0;
}

.slider {
  position: absolute;
  cursor: pointer;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background-color: #ccc;
  -webkit-transition: .4s;
  transition: .4s;
  background-color: #ca2222;
}

.slider:before {
  position: absolute;
  content: "";
  height: 26px;
  width: 26px;
  left: 4px;
  bottom: 4px;
  background-color: white;
  -webkit-transition: .4s;
  transition: .4s;
}

input:checked + .slider {
  background-color: #2ab934;
}

input:focus + .slider {
  box-shadow: 0 0 1px #2196F3;
}

input:checked + .slider:before {
  -webkit-transform: translateX(26px);
  -ms-transform: translateX(26px);
  transform: translateX(26px);
}

.slider.round {
  border-radius: 34px;
}

.slider.round:before {
  border-radius: 50%;
}
</style>

</head>

<body>

<form>

<input type="hidden" name="user" value='1' />

<h2>Program selection</h2>
<p><input type="submit" class="bttn" value="Mode 1" name="prog""/></p>
<p><input type="submit" class="bttn" value="Mode 2" name="prog""/></p>
<p><input type="submit" class="bttn" value="Mode 3" name="prog" /></p>
<p><input type="submit" class="bttn" value="Cruise Mode" name="prog" /></p>
<p><input type="submit" class="bttn" value="Rear Left Arrow" name="prog" /></p>
<p><input type="submit" class="bttn" value="Rear Right Arrow" name="prog" /></p>
<p><input type="submit" class="bttn" value="Take Downs" name="prog" /></p>

<h2>Modifiers</h2>

<p>
<label class="switch">
  <input type="checkbox" name="RightSide" onClick="this.form.submit()" value='1' {C1}>
  <span class="slider round"></span>
</label>
Enable right-side alley lights
</p>

<p>
<label class="switch">
  <input type="checkbox" name="LeftSide" onClick="this.form.submit()" value='1' {C2}>
  <span class="slider round"></span>
</label>
Enable left-side alley lights
</p>

<p>
<label class="switch">
  <input type="checkbox" name="Flashing" onClick="this.form.submit()" value='1' {C3}>
  <span class="slider round"></span>
</label>
Enable flashing
</p>

<p>
<label class="switch">
  <input type="checkbox" name="LowPower" onClick="this.form.submit()" value='1' {C4}>
  <span class="slider round"></span>
</label>
Low power
</p>

<p>
<label class="switch">
  <input type="checkbox" name="FlashTD" onClick="this.form.submit()" value='1' {C5}>
  <span class="slider round"></span>
</label>
Flash TDs and alleys
</p>

<p>
<label class="switch">
  <input type="checkbox" name="DisableFront" onClick="this.form.submit()" value='1' {C6}>
  <span class="slider round"></span>
</label>
Disable front lights
</p>

<p>
<label class="switch">
  <input type="checkbox" name="DisableRear" onClick="this.form.submit()" value='1' {C7}>
  <span class="slider round"></span>
</label>
Disable rear lights
</p>

<p>
<label class="switch">
  <input type="checkbox" name="ProgramMode" onClick="this.form.submit()" value='1' {C8}>
  <span class="slider round"></span>
</label>
Program mode
</p>

</form>

</body>
</html> 
 

)=====";

void setup(){
  Serial.begin(115200);
  Serial.println("=== Torrent LED Bar ===");
  Serial.println("Starting ...");
  
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
  //webServer.on("/", handleRoot);
  webServer.onNotFound(handleRoot);
  webServer.begin();
  Serial.println("OK !");
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}

void handleRoot() {  
  Serial.println("Processing request... ");
  handleSubmit();
  sendIndex();
}

void sendIndex(){
  Serial.println("Sending index page... ");
  String index_str(index_page);
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

void handleSubmit(){
  Serial.println("handleSubmit");

  if (webServer.hasArg("user"))  {
    if (webServer.hasArg("prog"))  {
      String prog = webServer.arg("prog");
      Serial.print("Prog= "); Serial.println(prog);
  
      if (prog == "Mode 1")           datah=0b01111111;
      if (prog == "Mode 2")           datah=0b10111111;
      if (prog == "Mode 3")           datah=0b11011111;
      if (prog == "Cruise Mode")      datah=0b11101111;
      if (prog == "Rear Left Arrow")  datah=0b11110111;
      if (prog == "Rear Right Arrow") datah=0b11111011;
      if (prog == "Take Downs")       datah=0b11111101;
    }
  
    C1 = webServer.hasArg("RightSide")    ? 1:0;
    C2 = webServer.hasArg("LeftSide")     ? 1:0;
    C3 = webServer.hasArg("Flashing")     ? 1:0;
    C4 = webServer.hasArg("LowPower")     ? 1:0;
    C5 = webServer.hasArg("FlashTD")      ? 1:0;
    C6 = webServer.hasArg("DisableFront") ? 1:0;
    C7 = webServer.hasArg("DisableRear")  ? 1:0;  
    C8 = webServer.hasArg("ProgramMode")  ? 1:0;  
  
    datah = datah & ~C1;
    datal = C2*128 + C3*64 + C4*32 + C5*16 + C6*8 + C7*4 + C8*2;
    datal = ~byte(datal);
    writeRegister(datah, datal);
  }
}
