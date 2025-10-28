#ifndef APPWEB_FUNCTIONS
#define APPWEB_FUNCTIONS

//***********************************
//************* LIBRAIRIES ESP
//***********************************
#ifdef  TTGO
  #include <TFT_eSPI.h>
  #include <SPI.h>
#endif
#ifdef ESP32D1MINI_FIRMWARE
  #include <OLEDDisplay.h>
  #include "OLEDDisplayUi.h"
  #include "SSD1306Wire.h"
#endif

//***********************************
//************* PROGRAMME PVROUTEUR
//***********************************
#include "energyFunctions.h"
#include "homeassistant.h"
#include "spiffsFunctions.h"
#include <RBDdimmer.h>
#include "unified_dimmer.h"
#include "minuteur.h"

//***********************************
//************* Variables externes
//***********************************
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
extern Programme programme_marche_forcee;
#ifdef  TTGO
  extern TFT_eSPI display ;   // Invoke library
#endif
#ifdef ESP32D1MINI_FIRMWARE
  extern SSD1306Wire display;
  extern OLEDDisplayUi   ui;
#endif

//***********************************
//************* Variables locales
//***********************************
String configweb; 
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
//************* getState_short
//************* retour des pages
//***********************************
String getState_short() {
  String state_short; 
  JsonDocument doc;
  doc["watt"] = int(gDisplayValues.watt);
  doc["dimmer"] = gDisplayValues.puissance_route;
  doc["temperature"] = gDisplayValues.temperature;
  serializeJson(doc, state_short);
  return String(state_short);
}

//***********************************
//************* getState
//***********************************
String getState() {
  String state=STABLE; 
  if (gDisplayValues.watt >= config.delta  ) {   
    state = GRID; 
  }
  if (gDisplayValues.watt <= config.deltaneg ) {   
    state = INJECTION; 
  }
 
  const String pvname = String("PV ROUTER ") + WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
  JsonDocument doc;
  doc["state"] = state;
  doc["watt"] = int(gDisplayValues.watt);
  doc["dimmer"] = gDisplayValues.puissance_route;
  doc["temperature"] = gDisplayValues.temperature;
  if (test_fs_version(false)) { 
    doc["version"] = VERSION ; 
  } 
  else { 
    doc["version"] = String(VERSION) + String(FS_UPDATE); 
  }
  doc["RSSI"] = WiFi.RSSI();
  doc["name"] =  String(pvname); 
  doc["dallas"] = dallas.lost;  //perte de la data dallas
  doc["minuteur"] = programme.run; 
  doc[ "security" ] = dallas.security;
  doc["relay1"]   = digitalRead(RELAY1);
  doc["relay2"]   = digitalRead(RELAY2);
  
  if (programme_marche_forcee.run) {
    doc["boost"] = programme_marche_forcee.run;
    doc["boost_endtime"] = programme_marche_forcee.heure_arret; 
  }
  doc["boost_max_temp"] = programme_marche_forcee.temperature;
  state=""; 
  serializeJson(doc, state);

  return String(state);
}

//***********************************
//************* getStateFull
//***********************************
String getStateFull() {
  String state=STABLE; 
  if (gDisplayValues.watt >= config.delta  ) {   
    state = GRID; 
  }
  if (gDisplayValues.watt <= config.deltaneg ) {   
    state = INJECTION; 
  }
 

  const String pvname = String("PV ROUTER ") + WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);

  JsonDocument doc;
  doc["state"] = state;
  doc["gDisplayValues.watt"] = int(gDisplayValues.watt);
  doc["unified_dimmer.get_power"]= unified_dimmer.get_power();
  doc["dimmer"] = gDisplayValues.puissance_route;
  doc["gDisplayValues.dimmer"]  = gDisplayValues.dimmer;

  doc["temperature"] = gDisplayValues.temperature;
  if (test_fs_version(false)) { 
    doc["version"] = VERSION ; 
  } 
  else { 
    doc["version"] = String(VERSION) + String(FS_UPDATE);
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

//***********************************
//************* stringbool
//***********************************
inline const char* stringBool(bool myBool) {
    return myBool ? "true" : "false";
}

inline const char* stringInt(bool myBool) {
    return myBool ? "1" : "0";
}

//***********************************
//************* getServermode
//***********************************
bool getServermode(String Servermode) {
  if ( Servermode == "screen" ) {  
    gDisplayValues.screenstate = !gDisplayValues.screenstate; return true;
  }
  if ( Servermode == "Jeedom" ) {   
    config.UseJeedom = !config.UseJeedom; return true;
  }
  if ( Servermode == "Autonome" ) {   
    config.autonome = !config.autonome; return true;
  }
  if ( Servermode == "dimmerlocal" ) {   
    config.dimmerlocal = !config.dimmerlocal;  
    //dimmer1.setPower(0);
    unified_dimmer.set_power(0);
    config.preheat = false;
    return true;
  }
  if ( Servermode == "MQTT" ) {   
    config.mqtt = !config.mqtt; return true;
  }
  if ( Servermode == "polarity" ) {   
    config.polarity = !config.polarity; config.sauve_polarity(); return true;
  }
  if ( Servermode == "envoy" ) {   
    configmodule.enphase_present = !configmodule.enphase_present; return true;
  }
  if ( Servermode == "fronius" ) {   
    configmodule.Fronius_present = !configmodule.Fronius_present; return true;
  }
  if ( Servermode == "TRI" ) {   
    config.Shelly_tri = !config.Shelly_tri; return true;
  }

  #ifndef LIGHT_FIRMWARE
    if ( Servermode == "HA" ) {   
      configmqtt.HA = !configmqtt.HA; 
      if (configmqtt.HA) 
        init_HA_sensor(); 
      return true;
    }
  #endif
  
  if ( Servermode == "flip" ) {   
    config.flip = !config.flip; 
    #ifdef  TTGO
      if (config.flip) 
      display.setRotation(3);
    else 
      display.setRotation(1);
    #endif
    #ifdef  ESP32D1MINI_FIRMWARE
    if (config.flip) 
        {
        display.flipScreenVertically();
        }
    else
        {
          display.init();
          display.display();
        }
    #endif
    return true;
  } 

  return false;
}

//***********************************
//************* getSigma
//***********************************
String getSigma() {
   return String(gDisplayValues.watt);
}

//***********************************
//************* getcosphi
//***********************************
String getcosphi() {
  String rawdata ="";
  
  return String(rawdata) ;
}
//***********************************
//************* getpuissance
//***********************************
String getpuissance() {
  int bestpuissance =1 ;

  return String(bestpuissance*config.facteur) ;
}

//***********************************
//************* getconfig
//***********************************
String getconfig() {
  String configweb; 
  char buffer[8];  // NOSONAR
  JsonDocument doc;

  doc["version"] = String(VERSION);
  doc["delta"] = config.delta_init;
  doc["deltaneg"] = config.deltaneg_init;
  doc["dimmer"] = config.dimmer;
  doc["cosphi"] = config.cosphi;
  doc["dimmerlocal"] = config.dimmerlocal;  
  dtostrf(config.facteur, 5, 2, buffer); 
  doc["facteur"] = buffer;
  doc["resistance"] = config.charge1;
  doc["resistance2"] = config.charge2;
  doc["resistance3"] = config.charge3;
  doc["polarity"] = config.polarity;
  doc["screentime"] = config.ScreenTime;
  doc["Fusiblelocal"] = config.localfuse;
  doc["maxtemp"] = config.tmax;
  doc["mintemp"] = config.tmin;
  doc["voltage"] = config.voltage;
  doc["offset"] = config.offset;
  doc["flip"] = config.flip;
  doc["SCT_13"] = config.SCT_13;
  doc["trigger"] = config.trigger;
      
  serializeJson(doc, configweb);

  return String(configweb);
}

//***********************************
//************* getenvoy
//***********************************
String getenvoy() {
  String retour ;
  JsonDocument doc;

  doc["IP_ENPHASE"] = configmodule.hostname;
  doc["PORT_ENPHASE"] = configmodule.port;
  doc["Type"] = configmodule.version;
  doc["version"] = configmodule.envoy;
  doc["token"] = configmodule.token;
  
  serializeJson(doc, retour);

  return String(retour) ;
}

//***********************************
//************* getwifi
//***********************************
String getwifi() {
  String retour ;
  JsonDocument doc;

  doc["ssid"] = configwifi.SID;
  doc["password"] = SECURITEPASS;
  doc["no_ap"] = config.NO_AP; // pour refuser le mode AP
  
  serializeJson(doc, retour);
  
  return String(retour) ;
}

//***********************************
//************* getmqtt
//***********************************
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

//***********************************
//************* getdebug
//***********************************
String getdebug() {
  configweb = "";
  configweb += "middle:" + String(middle_debug) + "\r\n" ; 

  //// calcul du cos phi par recherche milieu de demi onde positive 
  int start=0;int end=0;int debug_half=0;
  for ( int i=0; i < 72; i ++ ) {
    if ( porteuse[i] !=0  && start == 0 ) {
      start = i ;
    }
    if ( porteuse[i] ==0 && start != 0 && end == 0 ) {
      end = i ;
    }
    configweb += String(i) + "; "+ String(tableau[i]) + "; "+ String(porteuse[i]) + "\r\n" ;
  } // for
  
  debug_half = 36 - ( end - start ); 
  configweb += "  cosphi :" + String(debug_half) + "  end  :" + String(end ) +"  start :" + String(start)  ; 

  return String(configweb);
}

//***********************************
//************* getmemory
//***********************************
String getmemory() {
  String memory = "";

  return String(memory);
}

//***********************************
//************* injection_type
//***********************************
String injection_type() {
  String state = STABLE; 

  if (gDisplayValues.watt >= config.delta ) {   
    state = GRID; 
  }
  
  if (gDisplayValues.watt <= config.deltaneg ) {   
    state = INJECTION; 
  }
  
  return (state);
}

//***********************************
//************* injection_type
//***********************************
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

//***********************************
//************* serial_read
//***********************************
void serial_read() {
  String message_get =""; 
  int watchdog; 

  while (Serial.available() > 0 || watchdog > 255 ) {
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

    if (index != -1 ) {
      Serial.println("commande reboot reçue");
      savelogs("-- reboot Serial commande -- ");
      ESP.restart();
    }

    /// recupération du SSID
    index = message_get.indexOf("ssid");
    if (index != -1 ) {    
      char ssidArray[51]; // NOSONAR
      int ssidLength = message_get.length() - 4;  // Longueur du SSID à partir de l'index 5
      
      // Extraire le SSID de message_get
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
    } // if (index != -1 )

    // récupération du mot de passe
    index = message_get.indexOf("pass");
    if (index != -1 ) {
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
    } // if (index != -1 )

    index = message_get.indexOf("log");
    if (index != -1 ){
      logging.serial = true; 
      return; 
    }

    #ifdef TTGO
    index = message_get.indexOf("flip");
    if (index != -1 ){
      config.flip = !config.flip; 
      if (config.flip) 
        display.setRotation(3);
      else 
        display.setRotation(1);
      
        logging.Set_log_init(config.saveConfiguration().c_str(),true); ///save configuration
      return; 
    }
    #endif

    if (message_get.length() !=0) {
      Serial.println("Commande disponibles :");
      Serial.println("'reboot' pour redémarrer le routeur ");
      Serial.println("'pass' pour changer le mdp wifi ( doit être configuré avant le SSID )");
      Serial.println("'ssid' pour changer le SSID wifi");
      Serial.println("'log' pour afficher les logs serial");
      Serial.println("'flip' pour retourner l'ecran");
    }
  } // if (message_get.length() !=0 ) 
 }

#endif
