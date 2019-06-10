![In progress](https://img.shields.io/badge/status-in%20progress-orange.svg)

# Purpose
The purpose of this project is to create a Wifi controller for an orange Axixtech Torrent Led Light Bar.  

![](torrent.jpg)

# Realisation

According to [the manual](../master/TORRENT%20Lightbar%20Operation%20Manual.pdf) the wiring is extremely simple.  
To select a mode, one should just connect one of the wires to 12V.  

![](wiring.jpg).

## Electronics

The circuit is rather simple, based on Darlington NPN transistor arrays and shift registers.  
[My EDA project can be found here](https://easyeda.com/arnaud.dessein/torrent-light-bar)  

![](https://image.easyeda.com/histories/0bb19d9765f84e8ca09aadaa262d7e98.png)  
![](https://image.easyeda.com/histories/1ee4a7d05cbd480481dc6c2137dd454d.png)  

## Code

The ESP8266 will behave as an access point.
Once connected to it, the user can change the flashing modes on a web page. 
I will probably get inspiration from [this page](https://circuits4you.com/2016/12/16/esp8266-web-server-html/).
