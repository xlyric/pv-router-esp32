#ifndef WIFI_FUNCTIONS
#define WIFI_FUNCTIONS

#include <Arduino.h>
#include "WiFi.h"
#include "HTTPClient.h"


#include "../config/enums.h"
#include "../config/config.h"

const char *ssid = "PV-ROUTER";
const char *passphrase = "PV-ROUTER";

IPAddress local_IP(192,168,4,1);
IPAddress gateway(192,168,4,254);
IPAddress subnet(255,255,255,0);

extern Config config; 
extern Logs logging; 

extern HTTPClient httpdimmer;

void APConnect();
void WiFiEvent(WiFiEvent_t event);
String ipToString(IPAddress ip); 
bool dimmeradress(IPAddress dimmertemp );
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
void search_wifi_ssid();


void WIFIDimmerIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));
 

}

void APConnect() {
  if (!AP) {
    WiFi.onEvent(WiFiEvent);
    WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED);
    Serial.println(WiFi.macAddress());
    //String network = ssid + String('-') + WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
    Serial.println(gDisplayValues.pvname);
    
    int net_len = gDisplayValues.pvname.length() + 1; 
    char char_ssid[net_len];
    gDisplayValues.pvname.toCharArray(char_ssid, net_len);

    Serial.print("Setting AP-ROUTER configuration ... ");
    Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

    Serial.print("Setting AP-ROUTER ... ");
    Serial.println(WiFi.softAP(char_ssid,passphrase) ? "Ready" : "Failed!");
    
    Serial.print("Soft-AP IP address = ");
    Serial.println(WiFi.softAPIP());
    
    logging.Set_log_init("New connexion on AP :",true);
    logging.Set_log_init(String(WiFi.softAPIP()).c_str());
    logging.Set_log_init("\r\n");
    
    AP = true;
  }
}

void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);

  switch (event) {
    case ARDUINO_EVENT_WIFI_READY: 
      Serial.println("WiFi interface ready");
      break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
      Serial.println("Completed scan for access points");
      break;
    case ARDUINO_EVENT_WIFI_STA_START:
      Serial.println("WiFi client started");
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.println("WiFi clients stopped");
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("Disconnected from WiFi access point");
      if (AP) {
        savelogs( "-- sortie du mode AP -- ");
        ESP.restart();
      }
      WiFi.begin(ssid, passphrase);
      break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        Serial.println("Authentication mode of access point has changed");
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        Serial.print("Obtained IP address: ");
        Serial.println(WiFi.localIP());
        break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        Serial.println("Lost IP address and IP address is reset to 0");
        break;
    case ARDUINO_EVENT_WPS_ER_SUCCESS:
        Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
        break;
    case ARDUINO_EVENT_WPS_ER_FAILED:
        Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
        break;
    case ARDUINO_EVENT_WPS_ER_TIMEOUT:
        Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
        break;
    case ARDUINO_EVENT_WPS_ER_PIN:
        Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
        break;
    case ARDUINO_EVENT_WIFI_AP_START:
        Serial.println("WiFi access point started");
        break;
    case ARDUINO_EVENT_WIFI_AP_STOP:
        Serial.println("WiFi access point  stopped");
        break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
        Serial.println("Client connected");
        break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
        Serial.println("Client disconnected");
        break;
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
        Serial.println("Assigned IP address to client");
        break;
    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
        Serial.println("Received probe request");
        break;
    case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
        Serial.println("AP IPv6 is preferred");
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
        Serial.println("STA IPv6 is preferred");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP6:
        Serial.println("Ethernet IPv6 is preferred");
        break;
    case ARDUINO_EVENT_ETH_START:
        Serial.println("Ethernet started");
        break;
    case ARDUINO_EVENT_ETH_STOP:
        Serial.println("Ethernet stopped");
        break;
    case ARDUINO_EVENT_ETH_CONNECTED:
        Serial.println("Ethernet connected");
        break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        Serial.println("Ethernet disconnected");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        Serial.println("Obtained IP address");
        break;
    default: break;
}}


/*
* WiFi Events
0  SYSTEM_EVENT_WIFI_READY               < ESP32 WiFi ready
1  SYSTEM_EVENT_SCAN_DONE                < ESP32 finish scanning AP
2  SYSTEM_EVENT_STA_START                < ESP32 station start
3  SYSTEM_EVENT_STA_STOP                 < ESP32 station stop
4  SYSTEM_EVENT_STA_CONNECTED            < ESP32 station connected to AP
5  SYSTEM_EVENT_STA_DISCONNECTED         < ESP32 station disconnected from AP
6  SYSTEM_EVENT_STA_AUTHMODE_CHANGE      < the auth mode of AP connected by ESP32 station changed
7  SYSTEM_EVENT_STA_GOT_IP               < ESP32 station got IP from connected AP
8  SYSTEM_EVENT_STA_LOST_IP              < ESP32 station lost IP and the IP is reset to 0
9  SYSTEM_EVENT_STA_WPS_ER_SUCCESS       < ESP32 station wps succeeds in enrollee mode
10 SYSTEM_EVENT_STA_WPS_ER_FAILED        < ESP32 station wps fails in enrollee mode
11 SYSTEM_EVENT_STA_WPS_ER_TIMEOUT       < ESP32 station wps timeout in enrollee mode
12 SYSTEM_EVENT_STA_WPS_ER_PIN           < ESP32 station wps pin code in enrollee mode
13 SYSTEM_EVENT_AP_START                 < ESP32 soft-AP start
14 SYSTEM_EVENT_AP_STOP                  < ESP32 soft-AP stop
15 SYSTEM_EVENT_AP_STACONNECTED          < a station connected to ESP32 soft-AP
16 SYSTEM_EVENT_AP_STADISCONNECTED       < a station disconnected from ESP32 soft-AP
17 SYSTEM_EVENT_AP_STAIPASSIGNED         < ESP32 soft-AP assign an IP to a connected station
18 SYSTEM_EVENT_AP_PROBEREQRECVED        < Receive probe request packet in soft-AP interface
19 SYSTEM_EVENT_GOT_IP6                  < ESP32 station or ap or ethernet interface v6IP addr is preferred
20 SYSTEM_EVENT_ETH_START                < ESP32 ethernet start
21 SYSTEM_EVENT_ETH_STOP                 < ESP32 ethernet stop
22 SYSTEM_EVENT_ETH_CONNECTED            < ESP32 ethernet phy link up
23 SYSTEM_EVENT_ETH_DISCONNECTED         < ESP32 ethernet phy link down
24 SYSTEM_EVENT_ETH_GOT_IP               < ESP32 ethernet got IP from connected AP
25 SYSTEM_EVENT_MAX
*/

String ipToString(IPAddress ip){
  String s="";
  for (int i=0; i<4; i++)
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  return s;
}

bool dimmeradress(IPAddress dimmertemp ){
  const String baseurl = "/state" ; 
  String ipaddress= ipToString(dimmertemp);

  httpdimmer.begin(ipaddress,80,baseurl);   
  int httpResponseCode = httpdimmer.GET();  
  Serial.println(ipaddress);

  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    httpdimmer.end();
    ipaddress.toCharArray(config.dimmer, 16); 
    return true; 
  }
  httpdimmer.end();
  return false; 
}


void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  delay(1000); ///add delay for waiting start external web service
  IPAddress dimmertemp; 
  dimmertemp = info.got_ip.ip_info.ip.addr; 
  if (dimmeradress(dimmertemp)) {
    Serial.println("dimmer ");
        
        logging.Set_log_init("New Dimmer on AP",true);
  //  logging.start += "New Dimmer on AP :" + String(dimmertemp) +"\r\n";
  }
  else {
    Serial.println("not dimmer");
    
    logging.Set_log_init("New Connexion on AP",true);
// logging.start += "New Connexion on AP :" + String(dimmertemp) +"\r\n";
  }

  Serial.println(dimmertemp);
}

/// @brief callback for lost wifi connection
void search_wifi_ssid(){
    Serial.println("Scan des réseaux WiFi...");
    int numNetworks = WiFi.scanNetworks();
  if (numNetworks == 0) {
    Serial.println("Aucun réseau WiFi trouvé.");
  }
  else
  {
    Serial.print(numNetworks);
    Serial.println(" Recherche du wifi configuré :");
    // Find best signal
    int strength=INT_MIN;
    int id=-1;
    int channel=-1;
    for (int i = 0; i < numNetworks; i++) 
    {
      Serial.println(WiFi.SSID(i));
      if (!strcmp(configwifi.SID, WiFi.SSID(i).c_str()) )
      {
        if (id>=0)
        {
          if (WiFi.RSSI(i) > strength)
          {
            id = i;
            strength = WiFi.RSSI(i);
            channel = WiFi.channel(i);
          }
        }
        else
        {
          id = i;
          strength = WiFi.RSSI(i);
          channel = WiFi.channel(i);
        }
      }
    }
    if(id!=-1)
    {
      Serial.println(WiFi.SSID(id));      
      Serial.println("SSID trouvé reboot en cours");
      //ESP.restart();
      /// reconnection wifi
      AP=false;
      WiFi.begin(configwifi.SID, configwifi.passwd,channel); 
      delay(1500);
      /// test si le wifi est connecté sur le mon SSID, on déconnecte le mode AP  
      if(WiFi.status() == WL_CONNECTED)
      {      
        if (WiFi.SSID() == configwifi.SID)
        {
          Serial.println("WiFi connecté");
          WiFi.softAPdisconnect(true);
          ESP.restart();
        }    
      }
    }
    else
    {
      Serial.print("Reseau ");
      Serial.print(configwifi.SID);
      Serial.println(" non trouvé!");
    }
  }
}



#endif