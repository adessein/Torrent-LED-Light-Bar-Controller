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

![](https://image.easyeda.com/histories/842fc65e87f74eb69158350fe2f58f6e.png)  
![](https://image.easyeda.com/histories/a20f3e8650d344d2a72c066444794c6b.png)  

## Code

The ESP8266 will behave as an access point.
Once connected to it, the user can change the flashing modes on a web page. 
I will probably get inspiration from [this page](https://circuits4you.com/2016/12/16/esp8266-web-server-html/).
