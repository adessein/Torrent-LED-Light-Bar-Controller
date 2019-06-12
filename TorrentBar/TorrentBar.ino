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

IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

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

<!-- https://stackoverflow.com/a/10420584/6863846 -->
<input type="hidden" name="action" id="action" />

<script language="javascript" type="text/javascript">
  $(document).ready(function () {
    //when a submit button is clicked, put its name into the action hidden field
    $(":submit").click(function () { $("#action").val(this.name); $(this).closest("form").submit();});
  });
</script>
</head>

<body>

<form>

<h2>Program selection</h2>
<p><input type="submit" class="bttn" value="Mode 1" name="prog" /></p>
<p><input type="submit" class="bttn" value="Mode 2" name="prog" /></p>
<p><input type="submit" class="bttn" value="Mode 3" name="prog" /></p>
<p><input type="submit" class="bttn" value="Cruise mode" name="prog" /></p>
<p><input type="submit" class="bttn" value="Rear Left Arrow" name="prog" /></p>
<p><input type="submit" class="bttn" value="Rear Right Arrow" name="prog" /></p>
<p><input type="submit" class="bttn" value="Take Dowmns" name="prog" /></p>

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
  <input type="checkbox" name="HighPower" onClick="this.form.submit()" value='1' {C4}>
  <span class="slider round"></span>
</label>
High power
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
  <input type="checkbox" name="DisableR" onClick="this.form.submit()" value='1' {C7}>
  <span class="slider round"></span>
</label>
Disable read lights
</p>

</form>

</body>
</html> 

)=====";

void setup()
{
        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
        WiFi.softAP(ssid);

        Serial.begin(115200);
        
        // if DNSServer is started with "*" for domain name, it will reply with
        // provided IP to all DNS request
        dnsServer.start(DNS_PORT, "*", apIP);

        webServer.on("/", handleRoot);
        webServer.onNotFound(handleRoot);
  
        webServer.begin();
}

void loop()
{
        dnsServer.processNextRequest();
        webServer.handleClient();
}

void handleRoot()
{
  if (webServer.hasArg("prog") | 
      webServer.hasArg("RightSide") |
      webServer.hasArg("LeftSide") |
      webServer.hasArg("Flashing") |
      webServer.hasArg("HighPower") |
      webServer.hasArg("FlashTD") |
      webServer.hasArg("DisableFront") |
      webServer.hasArg("DisableR")
      ) {
    handleSubmit();
  }
  else {
    String index_str(index_page);
    index_str.replace("{C1}","");
    index_str.replace("{C2}","checked");
    index_str.replace("{C3}","0");
    index_str.replace("{C4}","checked");
    index_str.replace("{C5}","checked");
    index_str.replace("{C6}","0");
    index_str.replace("{C7}","0");
    webServer.send(200, "text/html", index_str.c_str());
  }
}

void handleSubmit()
{
  Serial.println("handleSubmit");

  if (webServer.hasArg("prog"))
  {
    Serial.println("prog");
    Serial.println(webServer.arg("prog"));
    }

  if (webServer.hasArg("RightSide"))
  {
    Serial.print("RightSide= ");
    Serial.println(webServer.arg("RightSide"));
  }
  
  /*
  if (LEDvalue == "1")
  {
    writeLED(true);
    server.send(200, "text/html", INDEX_HTML);
  }
  else if (LEDvalue == "0")
  {
    writeLED(false);
    server.send(200, "text/html", INDEX_HTML);
  }
  */
}
