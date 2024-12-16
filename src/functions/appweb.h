#ifndef APPWEB_FUNCTIONS
#define APPWEB_FUNCTIONS

#include "energyFunctions.h"
#include "homeassistant.h"
#include "spiffsFunctions.h"
#include <RBDdimmer.h>
#include "unified_dimmer.h"
#include "minuteur.h"

String configweb; 
extern DisplayValues gDisplayValues;
extern Config config; 
extern Configwifi configwifi; 
extern Mqtt configmqtt; 
extern Logs logging;
extern Configmodule configmodule; 
extern dimmerLamp dimmer1; 

extern gestion_puissance unified_dimmer; 
extern Dallas dallas;
extern Programme programme;

#ifdef  TTGO
#include <TFT_eSPI.h>
#include <SPI.h>
extern TFT_eSPI display ;   // Invoke library
#endif
int middleoscillo = 1800;

constexpr const char* PARAM_INPUT_1 = "disengage_dimmer"; /// paramettre de retour sendmode
constexpr const char* PARAM_INPUT_2 = "cycle"; /// paramettre de retour cycle
constexpr const char* PARAM_INPUT_3 = "readtime"; /// paramettre de retour readtime
constexpr const char* PARAM_INPUT_4 = "cosphi"; /// paramettre de retour cosphi
constexpr const char* PARAM_INPUT_save = "save"; /// paramettre de retour cosphi
constexpr const char* PARAM_INPUT_dimmer = "dimmer"; /// paramettre de retour cosphi
constexpr const char* PARAM_INPUT_server = "server"; /// paramettre de retour server domotique
constexpr const char* PARAM_INPUT_IDX = "idx"; /// paramettre de retour idx
constexpr const char* PARAM_INPUT_IDXdimmer = "idxdimmer"; /// paramettre de retour idx
constexpr const char* PARAM_INPUT_port = "port"; /// paramettre de retour port server domotique
constexpr const char* PARAM_INPUT_delta = "delta"; /// paramettre retour delta
constexpr const char* PARAM_INPUT_deltaneg = "deltaneg"; /// paramettre retour deltaneg
constexpr const char* PARAM_INPUT_fuse = "fuse"; /// paramettre retour fusible numérique
constexpr const char* PARAM_INPUT_API = "apiKey"; /// paramettre de retour apiKey
constexpr const char* PARAM_INPUT_servermode = "servermode"; /// paramettre de retour activation de mode server
constexpr const char* PARAM_INPUT_dimmer_power = "POWER"; /// paramettre de retour activation de mode server
constexpr const char* PARAM_INPUT_facteur = "facteur"; /// paramettre retour delta
constexpr const char* PARAM_INPUT_tmax = "tmax"; /// paramettre retour delta
constexpr const char* PARAM_INPUT_mqttserver = "mqttserver"; /// paramettre retour mqttserver
constexpr const char* PARAM_INPUT_reset = "reset"; /// paramettre reset
constexpr const char* PARAM_INPUT_publish = "publish"; /// paramettre publication mqtt


//***********************************
//** Oscillo mode creation du tableau de mesure pour le graph
//***********************************

String oscilloscope() {

  int timer = 0; 
  int temp;
  int signe; 
  int moyenne; 
  int oscillo_freqmesure = 40; 
  int sigma = 0;
  String retour = "[[";
  
  front();
  
  delayMicroseconds (config.cosphi*config.readtime); // correction décalage
  while ( timer < oscillo_freqmesure )
  {

  

  temp =  adc1_get_raw((adc1_channel_t)4); signe = adc1_get_raw((adc1_channel_t)5);
  moyenne = middleoscillo  + signe/50; 
  sigma += temp;

  /// mode oscillo graph 

  
  retour += String(timer) + "," + String(moyenne) + "," + String(temp) + "],[" ; 
  timer ++ ;
  delayMicroseconds (config.readtime);
  } 
  

  temp =  adc1_get_raw((adc1_channel_t)4); signe = adc1_get_raw((adc1_channel_t)5);
  moyenne = middleoscillo  + signe/50; 
  retour += String(timer) + "," + String(moyenne) + "," + String(temp) + "]]" ;
  middleoscillo = sigma / oscillo_freqmesure ;

return ( retour ); 
  
}


//***********************************
//************* retour des pages
//***********************************

String getState() {
  String state=STABLE; 
  if (gDisplayValues.watt >= config.delta  ) {   state = GRID; }
  if (gDisplayValues.watt <= config.deltaneg ) {   state = INJECTION; }
 
  const String fs_update = String("<br>!! FS pas à jour !!") ;
  const String pvname = String("PV ROUTER ") + WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
  JsonDocument doc;
  doc["state"] = state;
  doc["watt"] = int(gDisplayValues.watt);
  doc["dimmer"] = gDisplayValues.puissance_route;
  doc["temperature"] = gDisplayValues.temperature;
  if (test_fs_version(false)) { doc["version"] = VERSION ; 
  } else { doc["version"] = VERSION + fs_update; 
  }
  doc["RSSI"] = WiFi.RSSI();
  doc["name"] =  String(pvname); 
  doc["dallas"] = dallas.lost;  //perte de la data dallas
  doc["minuteur"] = programme.run; 
  doc[ "security" ] = dallas.security;
  doc["relay1"]   = digitalRead(RELAY1);
  doc["relay2"]   = digitalRead(RELAY2);

  state=""; 
  serializeJson(doc, state);
  return String(state);
}

String getStateFull() {
  String state=STABLE; 
  if (gDisplayValues.watt >= config.delta  ) {   state = GRID; }
  if (gDisplayValues.watt <= config.deltaneg ) {   state = INJECTION; }
 
  const String fs_update = String("<br>!! FS pas à jour !!") ;
  const String pvname = String("PV ROUTER ") + WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);

  JsonDocument doc;
  doc["state"] = state;
  doc["gDisplayValues.watt"] = int(gDisplayValues.watt);
  doc["unified_dimmer.get_power"]= unified_dimmer.get_power();
  doc["dimmer"] = gDisplayValues.puissance_route;
  doc["gDisplayValues.dimmer"]  = gDisplayValues.dimmer;

  doc["temperature"] = gDisplayValues.temperature;
  if (test_fs_version(false)) { doc["version"] = VERSION ; 
  } else { doc["version"] = VERSION + fs_update; 
  }
  doc["RSSI"] = WiFi.RSSI();
  doc["name"] =  String(pvname); 
  doc["dallas.lost"] = dallas.lost;  //perte de la data dallas
  doc["security"] = gDisplayValues.security;
  doc["dallas.security"] = dallas.security;
  


  state=""; 
  serializeJson(doc, state);
  return String(state);
}


String stringbool(bool mybool){
  String truefalse = "true";
  if (mybool == false ) {truefalse = "";}
  return String(truefalse);
  }

bool getServermode(String Servermode) {
  if ( Servermode == "screen" ) {  gDisplayValues.screenstate = !gDisplayValues.screenstate; return true;}
  if ( Servermode == "Jeedom" ) {   config.UseJeedom = !config.UseJeedom; return true;}
  if ( Servermode == "Autonome" ) {   config.autonome = !config.autonome; return true;}
  if ( Servermode == "dimmerlocal" ) {   
                    config.dimmerlocal = !config.dimmerlocal;  
                    //dimmer1.setPower(0);
                    unified_dimmer.set_power(0);
                    return true;
                    }
  if ( Servermode == "MQTT" ) {   config.mqtt = !config.mqtt; return true;}
  if ( Servermode == "polarity" ) {   config.polarity = !config.polarity; config.sauve_polarity(); return true;}
  if ( Servermode == "envoy" ) {   configmodule.enphase_present = !configmodule.enphase_present; return true;}
  if ( Servermode == "fronius" ) {   configmodule.Fronius_present = !configmodule.Fronius_present; return true;}
  if ( Servermode == "TRI" ) {   config.Shelly_tri = !config.Shelly_tri; return true;}

  #ifndef LIGHT_FIRMWARE
    if ( Servermode == "HA" ) {   configmqtt.HA = !configmqtt.HA; 
                      if (configmqtt.HA) init_HA_sensor(); 
                      return true;
                      }
  #endif
  
  if ( Servermode == "flip" ) {   
              config.flip = !config.flip; 
              #ifdef  TTGO
              if (config.flip) display.setRotation(3);
              else display.setRotation(1);
              #endif
              return true;
              }
 

return false;
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

   int bestpuissance =1 ;
  return String(bestpuissance*config.facteur) ;
}
//***********************************
String getconfig() {
  String configweb; 
  JsonDocument doc;
  doc["version"] = String(VERSION);
  doc["delta"] = config.delta;
  doc["deltaneg"] = config.deltaneg;
  doc["dimmer"] = config.dimmer;
  doc["cosphi"] = config.cosphi;
  doc["dimmerlocal"] = config.dimmerlocal;
  
  char buffer[8];  // NOSONAR
  dtostrf(config.facteur, 5, 2, buffer); 
  doc["facteur"] = buffer;

  doc["resistance"] = config.charge1;
  doc["resistance2"] = config.charge2;
  doc["resistance3"] = config.charge3;
  doc["polarity"] = config.polarity;
  doc["screentime"] = config.ScreenTime;
  doc["Fusiblelocal"] = config.localfuse;
  doc["maxtemp"] = config.tmax;
  doc["voltage"] = config.voltage;
  doc["offset"] = config.offset;
  doc["flip"] = config.flip;

  doc["SCT_13"] = config.SCT_13;
  doc["trigger"] = config.trigger;
  
  
  serializeJson(doc, configweb);
  return String(configweb);
}

String getenvoy() {
  String VERSION_http = String(VERSION) + " " + String(COMPILE_NAME) ; 
  configweb = String(config.IDXdimmer) + ";" + String(config.IDX) + ";"  +  String(VERSION_http) +";" + "middle" +";"+ config.delta +";"+config.cycle+";"+config.dimmer+";"+config.cosphi+";"+config.readtime +";"+stringbool(config.UseDomoticz)+";"+stringbool(config.UseJeedom)+";"+stringbool(config.autonome)+";"+config.apiKey+";"+stringbool(config.dimmerlocal)+";"+config.facteur+";"+stringbool(config.mqtt)+";"+config.mqttserver+ ";"  + String(config.Publish)+";"+config.deltaneg+";"+config.charge1+";"+config.polarity+";"+config.ScreenTime+";"+config.localfuse+";"+config.tmax+";"+config.voltage+";"+config.offset+";"+stringbool(config.flip)+";"+stringbool(configmqtt.HA);
  return String(configweb);
}
//***********************************
String getchart() {
  String retour ="" ;
  //ne sert à rien si enphase en route
  if (configmodule.enphase_present == false && configmodule.Fronius_present == false && ( strcmp(config.topic_Shelly,"none") == 0 || strcmp(config.topic_Shelly,"") == 0 )  ) {
    retour = oscilloscope() ;
  }
        return String(retour);
}
//***********************************
//***********************************
String getwifi() {
  String retour ;
  JsonDocument doc;
  doc["ssid"] = configwifi.SID;
  doc["password"] = SECURITEPASS;
  serializeJson(doc, retour);
  return String(retour) ;
}

String getmqtt() {
  String retour; 
  JsonDocument doc;
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
  doc["TRI"] = config.Shelly_tri;
  serializeJson(doc, retour);
  return String(retour) ;
}


String getdebug() {
  configweb = "";
  configweb += "middle:" + String(middle_debug) + "\r\n" ; 
  //// calcul du cos phi par recherche milieu de demi onde positive 
  int start=0;int end=0;int debug_half=0  ;
    for ( int i=0; i < 72; i ++ )
    {
      if ( porteuse[i] !=0  && start == 0 ) {start = i ;}
      if ( porteuse[i] ==0 && start != 0 && end == 0 ) {end = i ;}
	  configweb += String(i) + "; "+ String(tableau[i]) + "; "+ String(porteuse[i]) + "\r\n" ;
    }
    debug_half = 36 - ( end - start ); 
    configweb += "  cosphi :" + String(debug_half) + "  end  :" + String(end ) +"  start :" + String(start)  ; 

    return String(configweb);
  }

//***********************************
String getmemory() {
   String memory = "";
      return String(memory);
  }


String injection_type() {
      String state = STABLE; 
      if (gDisplayValues.watt >= config.delta ) {   state = GRID; }
      if (gDisplayValues.watt <= config.deltaneg ) {   state = INJECTION; }
      
      return (state);
}


bool detecterEspace(const char* chaine) {
    // Parcourir chaque caractère de la chaîne
    for (int i = 0; chaine[i] != '\0'; ++i) {
        // Vérifier si le caractère actuel est un espace
        if (chaine[i] == ' ') {
            // Un espace a été détecté, retourner vrai
            return true;
        }
    }
    // Aucun espace détecté, retourner faux
    return false;
}

void serial_read() {
  String message_get =""; 
  int watchdog; 

    while (Serial.available() > 0 || watchdog > 255 )
    {
      message_get = Serial.readStringUntil('\n');
      message_get.replace("\n","");
      message_get.replace("\r","");
      watchdog ++;
    }

    watchdog = 0;
    
    if (message_get.length() !=0 ) {
      /// test du message 
      // reinint du compteur de timeout

      int index = message_get.indexOf("reboot");
      if (index != -1 ){
        Serial.println("commande reboot reçue");
        savelogs("-- reboot Serial commande -- ");
        ESP.restart();
      }

/// recupération du SSID
      index = message_get.indexOf("ssid");
      if (index != -1 ){
        // Extraire le SSID de message_get
        char ssidArray[51]; // NOSONAR
        int ssidLength = message_get.length() - 4;  // Longueur du SSID à partir de l'index 5
        message_get.toCharArray(ssidArray, ssidLength, 5);
        // protection chaine vide ou négative >> SSID "AP" par defaut
        if (ssidLength <= 4 || detecterEspace(ssidArray)) { 
          Serial.println("SSID Mis en AP" );
          strcpy(configwifi.SID, "AP");
          configwifi.sauve_wifi();
          return; 
        }

        Serial.println(ssidLength);
        Serial.println("ssid enregistré: " + String(ssidArray));
        strcpy(configwifi.SID, ssidArray);
        configwifi.sauve_wifi(); 
        return;
      }

// récupération du mot de passe
      index = message_get.indexOf("pass");
      if (index != -1 ){
        char passArray[60];   // NOSONAR
        int passLength = message_get.length() - 4;  // Longueur du PASS à partir de l'index 5
        /// protection contre chaine vide ou négative
        if (passLength <= 0) { 
          Serial.println("password vide" );
          return; 
        }

        message_get.toCharArray(passArray, passLength, 5);
        Serial.println("password enregistré ");
        strcpy(configwifi.passwd, passArray);
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
                  logging.Set_log_init(config.saveConfiguration(),true); ///save configuration
        return; 
      }

      if (message_get.length() !=0){
        Serial.println("Commande disponibles :");
        Serial.println("'reboot' pour redémarrer le routeur ");
        Serial.println("'pass' pour changer le mdp wifi ( doit être configuré avant le SSID )");
        Serial.println("'ssid' pour changer le SSID wifi");
        Serial.println("'log' pour afficher les logs serial");
        Serial.println("'flip' pour retourner l'ecran");

      }
    }
 }

#endif