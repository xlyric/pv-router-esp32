#ifndef APPWEB_FUNCTIONS
#define APPWEB_FUNCTIONS

#include "energyFunctions.h"
#include "homeassistant.h"
#include "spiffsFunctions.h"

String configweb; 
extern DisplayValues gDisplayValues;
extern Config config; 
extern Configwifi configwifi; 
extern Mqtt configmqtt; 
extern Logs logging;
extern Configmodule configmodule; 
#ifdef  TTGO
#include <TFT_eSPI.h>
#include <SPI.h>
extern TFT_eSPI display ;   // Invoke library
#endif
int middleoscillo = 1800;

const char* PARAM_INPUT_1 = "disengage_dimmer"; /// paramettre de retour sendmode
const char* PARAM_INPUT_2 = "cycle"; /// paramettre de retour cycle
const char* PARAM_INPUT_3 = "readtime"; /// paramettre de retour readtime
const char* PARAM_INPUT_4 = "cosphi"; /// paramettre de retour cosphi
const char* PARAM_INPUT_save = "save"; /// paramettre de retour cosphi
const char* PARAM_INPUT_dimmer = "dimmer"; /// paramettre de retour cosphi
const char* PARAM_INPUT_server = "server"; /// paramettre de retour server domotique
const char* PARAM_INPUT_IDX = "idx"; /// paramettre de retour idx
const char* PARAM_INPUT_IDXdimmer = "idxdimmer"; /// paramettre de retour idx
const char* PARAM_INPUT_port = "port"; /// paramettre de retour port server domotique
const char* PARAM_INPUT_delta = "delta"; /// paramettre retour delta
const char* PARAM_INPUT_deltaneg = "deltaneg"; /// paramettre retour deltaneg
const char* PARAM_INPUT_fuse = "fuse"; /// paramettre retour fusible numérique
const char* PARAM_INPUT_API = "apiKey"; /// paramettre de retour apiKey
const char* PARAM_INPUT_servermode = "servermode"; /// paramettre de retour activation de mode server
const char* PARAM_INPUT_dimmer_power = "POWER"; /// paramettre de retour activation de mode server
const char* PARAM_INPUT_facteur = "facteur"; /// paramettre retour delta
const char* PARAM_INPUT_tmax = "tmax"; /// paramettre retour delta
const char* PARAM_INPUT_mqttserver = "mqttserver"; /// paramettre retour mqttserver
const char* PARAM_INPUT_reset = "reset"; /// paramettre reset
const char* PARAM_INPUT_publish = "publish"; /// paramettre publication mqtt


//***********************************
//** Oscillo mode creation du tableau de mesure pour le graph
//***********************************

String oscilloscope() {

 // int starttime,endtime; 
  int timer = 0; 
  int temp, signe, moyenne; 
  int freqmesure = 40; 
  int sigma = 0;
  String retour = "[[";
  
  if (configmodule.enphase_present == false && configmodule.Fronius_present == false) {front();}
  
  delayMicroseconds (config.cosphi*config.readtime); // correction décalage
  while ( timer < ( freqmesure ) )
  {

  
  //temp =  analogRead(ADC_INPUT); signe = analogRead(ADC_PORTEUSE);
  temp =  adc1_get_raw((adc1_channel_t)4); signe = adc1_get_raw((adc1_channel_t)5);
  moyenne = middleoscillo  + signe/50; 
  sigma += temp;
  //moyenne = moyenne + abs(temp - middle) ;
  /// mode oscillo graph 

  
  retour += String(timer) + "," + String(moyenne) + "," + String(temp) + "],[" ; 
  timer ++ ;
  delayMicroseconds (config.readtime);
  } 
  
  //temp =  analogRead(ADC_INPUT); signe = analogRead(ADC_PORTEUSE);
  temp =  adc1_get_raw((adc1_channel_t)4); signe = adc1_get_raw((adc1_channel_t)5);
  moyenne = middleoscillo  + signe/50; 
  retour += String(timer) + "," + String(moyenne) + "," + String(temp) + "]]" ;
  middleoscillo = sigma / freqmesure ;

return ( retour ); 
  
}


//***********************************
//************* retour des pages
//***********************************

String getState() {
  String state=STABLE; 
  if (gDisplayValues.watt >= config.delta  ) {   state = GRID; }
  if (gDisplayValues.watt <= config.deltaneg ) {   state = INJECTION; }
  //Serial.println(gDisplayValues.temperature);  
  if (gDisplayValues.temperature == "null" ) { gDisplayValues.temperature = "0";  }
  if (gDisplayValues.temperature == "" ) { gDisplayValues.temperature = "0";  }
  //Serial.println(gDisplayValues.temperature);  
  DynamicJsonDocument doc(128);
  doc["state"] = state;
  doc["watt"] = int(gDisplayValues.watt);
  doc["dimmer"] = gDisplayValues.puissance_route;
  doc["temperature"] = gDisplayValues.temperature;
  doc["version"] = VERSION;
  state=""; 
  serializeJson(doc, state);
  return String(state);
}


String stringbool(bool mybool){
  String truefalse = "true";
  if (mybool == false ) {truefalse = "";}
  return String(truefalse);
  }
//***********************************
/*
String getSendmode() {
  String sendmode;
  if ( config.sending == 0 ) {   sendmode = "Off"; }
  else {   sendmode = "On"; }
  return String(sendmode);
}*/
//***********************************
String getServermode(String Servermode) {
  if ( Servermode == "screen" ) {  gDisplayValues.screenstate = !gDisplayValues.screenstate; }
  if ( Servermode == "Jeedom" ) {   config.UseJeedom = !config.UseJeedom;}
  if ( Servermode == "Autonome" ) {   config.autonome = !config.autonome; }
  if ( Servermode == "Dimmer local" ) {   config.dimmerlocal = !config.dimmerlocal; }
  if ( Servermode == "MQTT" ) {   config.mqtt = !config.mqtt; }
  if ( Servermode == "polarité" ) {   config.polarity = !config.polarity; }
  if ( Servermode == "envoy" ) {   configmodule.enphase_present = !configmodule.enphase_present; }
  if ( Servermode == "frontius" ) {   configmodule.Fronius_present = !configmodule.Fronius_present; }

  #ifndef LIGHT_FIRMWARE
    if ( Servermode == "HA" ) {   configmqtt.HA = !configmqtt.HA; 
                      if (configmqtt.HA) init_HA_sensor(); 
                      }
  #endif
  
  if ( Servermode == "flip" ) {   
              config.flip = !config.flip; 
              #ifdef  TTGO
              if (config.flip) display.setRotation(3);
              else display.setRotation(1);
              #endif
              }
  

return String(Servermode);
}
//***********************************
String getSigma() {
   return String(gDisplayValues.watt);
}

//***********************************
String getcosphi() {
  String rawdata ="";
   return String(rawdata) ;
}
//***********************************
String getpuissance() {
  //int tempvalue=15;
   int bestpuissance =1 ;
  return String(bestpuissance*config.facteur) ;
}
//***********************************
String getconfig() {
  String configweb; 
  DynamicJsonDocument doc(512);
  doc["Fusible"] = config.num_fuse;
  doc["version"] = String(VERSION);
  doc["delta"] = config.delta;
  doc["deltaneg"] = config.deltaneg;
  doc["dimmer"] = config.dimmer;
  doc["cosphi"] = config.cosphi;
  doc["dimmerlocal"] = config.dimmerlocal;
  
  char buffer[8];
  dtostrf(config.facteur, 5, 2, buffer); 
  doc["facteur"] = buffer;

  doc["resistance"] = config.resistance;
  doc["polarity"] = config.polarity;
  doc["screentime"] = config.ScreenTime;
  doc["Fusiblelocal"] = config.localfuse;
  doc["maxtemp"] = config.tmax;
  doc["voltage"] = config.voltage;
  doc["offset"] = config.offset;
  doc["flip"] = config.flip;
  doc["relaystart"] = config.relayon;
  doc["relaystop"] = config.relayoff;
  
  serializeJson(doc, configweb);
  return String(configweb);
}

String getenvoy() {
  configweb = String(config.IDXdimmer) + ";" +  config.num_fuse + ";"  + String(config.IDX) + ";"  +  String(VERSION) +";" + "middle" +";"+ config.delta +";"+config.cycle+";"+config.dimmer+";"+config.cosphi+";"+config.readtime +";"+stringbool(config.UseDomoticz)+";"+stringbool(config.UseJeedom)+";"+stringbool(config.autonome)+";"+config.apiKey+";"+stringbool(config.dimmerlocal)+";"+config.facteur+";"+stringbool(config.mqtt)+";"+config.mqttserver+ ";"  + String(config.Publish)+";"+config.deltaneg+";"+config.resistance+";"+config.polarity+";"+config.ScreenTime+";"+config.localfuse+";"+config.tmax+";"+config.voltage+";"+config.offset+";"+stringbool(config.flip)+";"+stringbool(configmqtt.HA)+";"+config.relayon+";"+config.relayoff;
  return String(configweb);
}
//***********************************
String getchart() {
  String retour ="" ;
    retour = oscilloscope() ;
      return String(retour);
}
//***********************************
//***********************************
String getwifi() {
  String retour =String(configwifi.SID) + ";" + String(SECURITEPASS)  ;
  return String(retour) ;
}

String getmqtt() {
  String retour; 
  DynamicJsonDocument doc(512);
  doc["server"] = config.mqttserver;
  doc["topic"] = config.Publish;
  doc["user"] = configmqtt.username;
  doc["password"] = SECURITEPASS;
  doc["MQTT"] = config.mqtt;
  doc["IDX"] = config.IDX;
  doc["IDXDIMMER"] = config.IDXdimmer;
  doc["port"] = config.mqttport;
  doc["HA"] = configmqtt.HA;
  doc["EM"] = config.topic_Shelly;
  doc["IDXDALLAS"] = config.IDXdallas;
  serializeJson(doc, retour);
  return String(retour) ;
}


String getdebug() {
  configweb = "";
  configweb += "middle:" + String(middle_debug) + "\r\n" ; 
  //// calcul du cos phi par recherche milieu de demi onde positive 
  int start=0;int end=0;int half=0  ;
    for ( int i=0; i < 72; i ++ )
    {
      if ( porteuse[i] !=0  && start == 0 ) {start = i ;}
      if ( porteuse[i] ==0 && start != 0 && end == 0 ) {end = i ;}
	  configweb += String(i) + "; "+ String(tableau[i]) + "; "+ String(porteuse[i]) + "\r\n" ;
    }
    half = 36 - ( end - start ); 
    configweb += "  cosphi :" + String(half) + "  end  :" + String(end ) +"  start :" + String(start)  ; 


    return String(configweb);
  }

//***********************************
String getmemory() {
   String memory = "";
   // memory = String(ESP.getFreeHeap()) + ";" + String(ESP.getHeapFragmentation()) ;
      return String(memory);
  }

//***********************************
String getlogs() {
    logging.start = logging.init + logging.start  + "}1"; 
    logging.power = true ; logging.sct = true; logging.sinus = true; 
    return logging.start ; 
  }

//***********************************
/*String processor(const String& var){
   Serial.println(var);
   if (var == "SIGMA"){
    return getSigma();
  }
  if (var == "STATE"){
    return getState();
  }
  if (var == "VERSION"){
    return VERSION;
  }
    

return getState();
}
*/

//***********************************
//************* Fonction domotique 
//***********************************

void SendToDomotic(String Svalue){
  String baseurl; 
  Serial.print("connecting to mqtt & dimmer");
  Serial.println(config.hostname);
  
 /* if ( config.mqtt == 1 ) {     mqtt(config.IDX,Svalue);  }*/
  Serial.println(baseurl);

  // http.begin(config.hostname,config.port,baseurl);
  //int httpCode = http.GET();
  //Serial.println("closing connection");
  //http.end();
/*
  if ( config.autonome == 1 && change == 1   ) {  baseurl = "/?POWER=" + String(dimmer_power) ; http.begin(config.dimmer,80,baseurl);   int httpCode = http.GET();
    http.end(); 
    if ( config.mqtt == 1 ) { mqtt(config.IDXdimmer, String(dimmer_power));  }
    delay (3000); // delay de transmission réseau dimmer et application de la charge
    */  
    
    //}


 
}

/*
void mqtt(String idx, String value)
{
  String nvalue = "0" ; 
  if ( value != "0" ) { nvalue = "2" ; }
String message = "  { \"idx\" : " + idx +" ,   \"svalue\" : \"" + value + "\",  \"nvalue\" : " + nvalue + "  } ";

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  client.publish("domoticz/in", String(message).c_str(), true);
  
}*/


String injection_type() {
      String state = STABLE; 
      if (gDisplayValues.watt >= config.delta ) {   state = GRID; }
      if (gDisplayValues.watt <= config.deltaneg ) {   state = INJECTION; }
      
      return (state);
}

/*
*  récupération de la température sur le dimmer 
*/
String Dimmer_temp(char* host) {
WiFiClient client;
  
  String url = "/state";
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");
  
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('OK');

  return (line);
   
}
/* //// la fonction prend 100 octets de plus .
void processMessage(String message_get ) {
  
    if (-1 == message_get.indexOf("reboot")) {
          Serial.println("commande reboot reçue");
          ESP.restart();
    } else if (-1 == message_get.indexOf("ssid")) {
          String wifitemp=message_get.substring(5, message_get.length());
          Serial.println("ssid enregistré: " + wifitemp);
          wifitemp.toCharArray(configwifi.SID,50);
          configwifi.sauve_wifi(); 
          return;
    } else if (-1 == message_get.indexOf("pass")) {
        Serial.println("password enregistré :");
        String passtemp=message_get.substring(5, message_get.length());
        passtemp.toCharArray(configwifi.passwd,50);
        configwifi.sauve_wifi(); 
        return;
    } else if (-1 == message_get.indexOf("log")) {
          logging.serial = true; 
          return; 
    } else if (-1 == message_get.indexOf("flip")) {
          config.flip = !config.flip; 
          if (config.flip) display.setRotation(3);
          else display.setRotation(1);
          saveConfiguration(filename_conf, config); 
          return; 
    } else if (message_get.length() !=0) {
          Serial.println("Commande disponibles :");
          Serial.println("'reboot' pour redémarrer le routeur ");
          Serial.println("'ssid' pour changer le SSID wifi");
          Serial.println("'pass' pour changer le mdp wifi");
          Serial.println("'log' pour afficher les logs serial");
          Serial.println("'flip' pour retourner l'ecran");
    }
  
}

*/
void serial_read() {
  String message_get; 
  int watchdog; 

    while (Serial.available() > 0 || watchdog > 255 )
    {
      message_get = Serial.readStringUntil('\n');
      message_get.replace("\n","");
      message_get.replace("\r","");
      watchdog ++;
    }

    if (message_get.length() !=0 ) {
      /// test du message 
      int index = message_get.indexOf("reboot");
      if (index != -1 ){
        Serial.println("commande reboot reçue");
        ESP.restart();
      }

      index = message_get.indexOf("ssid");
      if (index != -1 ){
        String wifitemp=message_get.substring(5, message_get.length());
        Serial.println("ssid enregistré: " + wifitemp);
        wifitemp.toCharArray(configwifi.SID,50);
        configwifi.sauve_wifi(); 
        return;
      }

      index = message_get.indexOf("pass");
      if (index != -1 ){
        Serial.println("password enregistré :");
        String passtemp=message_get.substring(5, message_get.length());
        passtemp.toCharArray(configwifi.passwd,50);
        configwifi.sauve_wifi(); 
        return;
      }

      index = message_get.indexOf("log");
      if (index != -1 ){
        logging.serial = true; 
        return; 
      }


      index = message_get.indexOf("flip");
      if (index != -1 ){
                  config.flip = !config.flip; 
                  if (config.flip) display.setRotation(3);
                  else display.setRotation(1);
                  saveConfiguration(filename_conf, config); 
        return; 
      }

   /*   index = message_get.indexOf("HA");
      if (index != -1 ){
                  configmqtt.HA = !configmqtt.HA; 
                  savemqtt(mqtt_conf, configmqtt);
                  Serial.print("/r/n MQTT pour HA est maintenant :");
                  Serial.println(String(configmqtt.HA).c_str());
        return; 
      }*/

      if (message_get.length() !=0){
        Serial.println("Commande disponibles :");
        Serial.println("'reboot' pour redémarrer le routeur ");
        Serial.println("'ssid' pour changer le SSID wifi");
        Serial.println("'pass' pour changer le mdp wifi");
        Serial.println("'log' pour afficher les logs serial");
        Serial.println("'flip' pour retourner l'ecran");
       // Serial.println("'HA' pour activer ou désactiver MQTT pour Home assistant");
      }
    }
 }

#endif