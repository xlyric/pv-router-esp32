# Pv router Version 1.3 for ESP32 or TTGO T display 

## TTGO version 
<img src="./images/routeur.jpg" align="up" height="300"  >
I created this pv router to separate the power part of the analysis part. 
After 2 years of working with an ESP8266, I made evolutions to upgrade to an ESP32. ( and visual Studio Code )
first time I use Oled display and I migrate to a TTGO tdisplay, for an easyer intregration. 

this router use a very simple tech by using phase differences.
The project can reproduce easily for the electronic board.
however I made an industrial version of it to make things easier.
<a href="https://www.helloasso.com/associations/apper/formulaires/4">The board was sold by a French Association ( APPER ) </a>

You only need to by separatly :
 . A <a href="https://amzn.to/3hVCLpf">TTGO-Tdisplay</a>
 . A <a href="https://amzn.to/3CtCHqi">SCT-013-30</a>
 . And an 12V - AC power supply ( Sinus ) you can made it with a modified old 12DC coil transformer. ( see below )

For packaging, I use the box that comes with the TTGO.

# Prerequiement : 
A circuit board has been created and is available ( tips ) 
<img src="https://nsa40.casimages.com/img/2019/09/05/190905103700235594.png">

<img src="https://nsa40.casimages.com/img/2019/08/22/190822020621726681.jpg">
                                                                           
<img src="https://nsa40.casimages.com/img/2019/08/22/190822020621896704.png">

# Installation : 
## configure Wifi 
rename the file wifi.json.ori to wifi.json and change SID 
## upload
if you know your configuration you can change it on config.json

Upload the firmware and filesystem. the system will start and working

## OTA UPload 
You can use OTA upload by the web page /update 
Build firmware of filesystem with VS and upload the .bin





## ESP32 Version or DIY version
connections : 
** ESP32  ** 
OLED  : 
3.3V 
GND 
21 SCL
22 SDA 

SCT013 
div bridge R/R  between  3.3V and GND
SCT013  connected at the middle of div bridge and pin 34 ( --> Linky )
condensator 470uf between GND and the middle of div bridge. ( filter )
***********

** TTGO ** 
pin 32 GRID
pin 33 SCT013
******** 

The carrier is made with a 12v transformer power supply.
The bridge rectifier is removed and the wavy part is kept
There is a divider bridge not to exceed 3.3V and a diode for half alternation.

The assembly as a whole will raise the power and the direction of the current.
Like the ESP8266 version, it can connect directly to the Wifi dimmer to control the power.
<a href="https://github.com/xlyric/PV-discharge-Dimmer-AC-Dimmer-KIT-Robotdyn">github Project</a>

# preparing power: 
Open the transformer and remove the diode bridge

<img src ="https://nsa40.casimages.com/img/2019/06/14/190614104905615784.jpg">

Close the transformer and verify the voltage. ~12V AC ( 9-16v ) and adjust the divider bridge if needed ( output 3.3V max )




