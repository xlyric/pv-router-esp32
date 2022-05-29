# Pv router Version 1.3 for ESP32 or TTGO T display 

## TTGO version 
<img src="./img/routeur.jpg" align="up" height="300"  >
I created this pv router to separate the power part of the analysis part. <br>
After 2 years of working with an ESP8266, I made evolutions to upgrade to an ESP32. ( and visual Studio Code )<br>
first time I use Oled display and I migrate to a TTGO tdisplay, for an easyer intregration. <br>

this router use a very simple tech by using phase differences.<br>
The project can reproduce easily for the electronic board.<br>
however I made an industrial version of it to make things easier.<br>
<a href="https://www.helloasso.com/associations/apper/formulaires/4">The board was sold by a French Association ( APPER ) </a>

You only need to by separatly : <br>
 . A <a href="https://amzn.to/3hVCLpf">TTGO-Tdisplay</a><br>
 . A <a href="https://amzn.to/3CtCHqi">SCT-013-30</a><br>
 . And an 12V - AC power supply ( Sinus ) you can made it with a modified old 12DC coil transformer. ( see below )<br>

For packaging, I use the box that comes with the TTGO.

<img src="./img/index.png" align="down" height="300"  > 

# Installation : 
## Configure Wifi 
Rename the file wifi.json.ori to wifi.json and change SID <br>
Rename the file config.json.ori to config.json 

## Upload
If you know your configuration you can change it on config.json<br>
Upload the firmware and filesystem. the system will start and working<br>
A complete documentation (in French ) is <a href="./Doc%20installation.pdf"> here </a>

## OTA UPload 
You can use OTA upload by the web page /update <br>
Build firmware of filesystem with VS and upload the .bin<br>

## Main Configuration 
Under the /setup.html page you can configure information for connecting to the MQTT server ( support domoticz or Jeedom ).<br>
Enter the information of the dimmer, the max limit percent power you want to send, the slot between the dimmer work ( low and high )<br>
If you want to swith off oled after a limited time ( button right for swith / or website )<br>
...<br>

<img src="./img/setup.png" align="down" height="300"  >

### Case of multi dimmers

In case you use multi dimmers, you need to calculate the max % needed for all dimmers, <br>
For example if 1st dimmer is 80% max and 2nd dimmer is 40% max, you need to configure the power limit to 120 <br>
( and configure all dimmers directly with the limit )<br>


## ESP32 Version or DIY version
connections : 
### ESP32  
OLED  : <br>
3.3V <br>
GND <br>
21 SCL<br>
22 SDA <br>

SCT013 wbr
div bridge R/R  between  3.3V and GND<br>
SCT013  connected at the middle of div bridge and pin 34 ( --> Linky )<br>
Condensator 470uf between GND and the middle of div bridge. ( filter )<br>


### TTGO 
pin 32 GRID <br>
pin 33 SCT013 <br>


The carrier is made with a 12v transformer power supply.<br>
The bridge rectifier is removed and the wavy part is kept<br>
There is a divider bridge not to exceed 3.3V and a diode for half alternation.<br>

The assembly as a whole will raise the power and the direction of the current.<br>
Like the ESP8266 version, it can connect directly to the Wifi dimmer to control the power.<br>
<a href="https://github.com/xlyric/PV-discharge-Dimmer-AC-Dimmer-KIT-Robotdyn">github Project</a><br>

# Preparing power: 
Open the transformer and remove the diode bridge

<img src ="https://nsa40.casimages.com/img/2019/06/14/190614104905615784.jpg">

Close the transformer and verify the voltage. ~12V AC ( 9-16v ) and adjust the divider bridge if needed ( output 3.3V max )


## Debug

you can show the signal with the oscillo on the main page
<img src="./img/oscillo.png" align="down" height="300"  >


###BUG DALLAS
Onboard Dallas not working on GPIO 37 on TTGO --> connect Dallas Data on GPIO 25