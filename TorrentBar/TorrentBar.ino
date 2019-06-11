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

const char INDEX_HTML[] =
"<!DOCTYPE html>\n<html>\n<head>\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" charset=\"utf-8\" />\n<style>\nbody {\n  font-family: Sans-serif;\n}\n\ninput[class=\"bttn\"],p{\n    font-size:24px;\n}\n\n.switch {\n  position: relative;\n  display: inline-block;\n  width: 60px;\n  height: 34px;\n}\n\n.switch input { \n  opacity: 0;\n  width: 0;\n  height: 0;\n}\n\n.slider {\n  position: absolute;\n  cursor: pointer;\n  top: 0;\n  left: 0;\n  right: 0;\n  bottom: 0;\n  background-color: #ccc;\n  -webkit-transition: .4s;\n  transition: .4s;\n  background-color: #ca2222;\n}\n\n.slider:before {\n  position: absolute;\n  content: \"\";\n  height: 26px;\n  width: 26px;\n  left: 4px;\n  bottom: 4px;\n  background-color: white;\n  -webkit-transition: .4s;\n  transition: .4s;\n}\n\ninput:checked + .slider {\n  background-color: #2ab934;\n}\n\ninput:focus + .slider {\n  box-shadow: 0 0 1px #2196F3;\n}\n\ninput:checked + .slider:before {\n  -webkit-transform: translateX(26px);\n  -ms-transform: translateX(26px);\n  transform: translateX(26px);\n}\n\n.slider.round {\n  border-radius: 34px;\n}\n\n.slider.round:before {\n  border-radius: 50%;\n}\n</style>\n\n<!-- https://stackoverflow.com/a/10420584/6863846 -->\n<script src=\"jquery.min.js\"></script>\n<input type=\"hidden\" name=\"action\" id=\"action\" />\n<script language=\"javascript\" type=\"text/javascript\">\n  \n  $(document).ready(function () {\n  //when a submit button is clicked, put its name into the action hidden field\n    $(\":checkbox\").click(function() { $(this).closest(\"form\").submit();});\n    $(\":submit\").click(function () { $(\"#action\").val(this.name); $(this).closest(\"form\").submit();});\n  });\n</script>\n</head>\n\n<body>\n\n<form>\n\n<h2>Program selection</h2>\n<p><input type=\"submit\" class=\"bttn\" value=\"Mode 1\" name=\"prog\" /></p>\n<p><input type=\"submit\" class=\"bttn\" value=\"Mode 2\" name=\"prog\" /></p>\n<p><input type=\"submit\" class=\"bttn\" value=\"Mode 3\" name=\"prog\" /></p>\n<p><input type=\"submit\" class=\"bttn\" value=\"Cruise mode\" name=\"prog\" /></p>\n<p><input type=\"submit\" class=\"bttn\" value=\"Rear Left Arrow\" name=\"prog\" /></p>\n<p><input type=\"submit\" class=\"bttn\" value=\"Rear Right Arrow\" name=\"prog\" /></p>\n<p><input type=\"submit\" class=\"bttn\" value=\"Take Dowmns\" name=\"prog\" /></p>\n\n<h2>Toggle Switch</h2>\n\n<p>\n<label class=\"switch\">\n  <input type=\"checkbox\" name=\"RightSide\">\n  <span class=\"slider round\"></span>\n</label>\nEnable right-side alley lights\n</p>\n\n<p>\n<label class=\"switch\">\n  <input type=\"checkbox\" name=\"LeftSide\">\n  <span class=\"slider round\"></span>\n</label>\nEnable left-side alley lights\n</p>\n\n<p>\n<label class=\"switch\">\n  <input type=\"checkbox\" name=\"Flashing\">\n  <span class=\"slider round\"></span>\n</label>\nEnable flashing\n</p>\n\n<p>\n<label class=\"switch\">\n  <input type=\"checkbox\" name=\"HighPower\">\n  <span class=\"slider round\"></span>\n</label>\nHigh power\n</p>\n\n<p>\n<label class=\"switch\">\n  <input type=\"checkbox\" name=\"FlashTD\">\n  <span class=\"slider round\"></span>\n</label>\nFlash TDs and alleys\n</p>\n\n<p>\n<label class=\"switch\">\n  <input type=\"checkbox\" name=\"DisableFront\">\n  <span class=\"slider round\"></span>\n</label>\nDisable front lights\n</p>\n\n<p>\n<label class=\"switch\">\n  <input type=\"checkbox\" name=\"DisableR\">\n  <span class=\"slider round\"></span>\n</label>\nDisable read lights\n</p>\n\n</form>\n\n</body>\n</html> \n"
;

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
    webServer.send(200, "text/html", INDEX_HTML);
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
