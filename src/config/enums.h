#ifndef ENUMS
#define ENUMS

#include <Arduino.h>
#ifndef LIGHT_FIRMWARE
  #include <PubSubClient.h>
  extern PubSubClient client;
#endif

#include <Preferences.h> 
#include <TimeLib.h>
#include <NTPClient.h>


#define SECURITEPASS "MyPassword"


// The state in which the device can be. This mainly affects what
// is drawn on the display.
enum DEVICE_STATE {
  CONNECTING_WIFI,
  CONNECTING_AWS,
  FETCHING_TIME,
  UP,
};

// Place to store all the variables that need to be displayed.
// All other functions should update these!
struct DisplayValues {
  double watt;
  bool wattIsValid; // Jeton pour measureEletricity.h permettant d'appliquer la puissance au dimmer (local ou distant)
  double amps;
  int8_t wifi_strength;
  DEVICE_STATE currentState;
  String IP;
  String time;
  bool injection; 
  int dimmer;
  int security; 
  int change;
  bool task;
  bool porteuse; 
  bool screenstate; 
  bool screenbutton;
  bool nextbutton;
  int option=0;
  float temperature = 0.0 ;
  int Fronius_conso; 
  int Fronius_prod; 
  int Fronius_totalconso;
  float celsius;
  int page=1; 
  int Shelly=-1;
  double enp_prod_whLifetime;
  double enp_cons_whLifetime;
  double enp_current_power_consumption;
  double enp_current_power_production;
  int puissance_route=0;
  bool dimmer_disengaged=false;
  const String pvname = "PV ROUTER " + WiFi.macAddress().substring(12,14) + WiFi.macAddress().substring(15,17);
  int serial_timeout = 0 ; ///arret du service serial après x loop d'inactivité
};

struct Config {
  char hostname[16];  // à vérifier si on peut pas le supprimer
  int port;  // idem  
  char apiKey[64];  // clé pour jeedom
  bool UseDomoticz;
  bool UseJeedom;
  int IDX;  // IDX pour domoticz
  int IDXdallas; // IDX pour domoticz
  char otapassword[64];
  int delta; 
  int deltaneg;
  int cosphi; // plus utilisé
  int readtime;  // temps de lecture des capteurs
  int cycle;  // cycle de lecture des capteurs
  bool sending; 
  bool autonome; // si dimmer en local 
  char dimmer[16];  // adresse IP du dimmer
  bool dimmerlocal; // si dimmer en local
  float facteur; // facteur de correction de la puissance
  int num_fuse;
  int localfuse;
  int tmax;
  bool mqtt;
  char mqttserver[16];
  int mqttport; 
  int IDXdimmer;
  int resistance;  // résistance de la charge
  bool polarity; 
  char Publish[100];
  int  ScreenTime;
  int voltage; 
  int offset; 
  bool flip;
  int relayon; 
  int relayoff;
  bool restart;
  char topic_Shelly[100]; 
  bool Shelly_tri;
  int SCT_13=30;
/// @brief  // Puissance de la charge 2 déclarée dans la page web
  int charge2; 
/// @brief  // Puissance de la charge 3 déclarée dans la page web
  int charge3;
/// @brief  // Somme des 3 charges déclarées dans la page web
  int charge;

  Preferences preferences;

public:
  bool sauve_polarity() {
  preferences.begin("polarity", false);
  preferences.putBool("polarity",polarity);
  preferences.end();
  return true; 
  }

  bool recup_polarity() {
  preferences.begin("polarity", false);
  polarity = preferences.getBool("polarity", false);
  preferences.end();
  return true; 
  }
  
  void calcul_charge() {
    charge = resistance + charge2 + charge3;
  }
};

struct Configwifi {
  
  Preferences preferences;

  char SID[32];
  char passwd[64];

  public:bool sauve_wifi() {
  preferences.begin("credentials", false);
  preferences.putString("ssid",SID);
  preferences.putString("password",passwd);
  preferences.end();
  return true; 
  }

  public:bool recup_wifi() {
  preferences.begin("credentials", false);
  String tmp; 
  tmp = preferences.getString("ssid", "AP");
  tmp.toCharArray(SID,32);
  tmp = preferences.getString("password", "");
  tmp.toCharArray(passwd,64);
  preferences.end();
  if (strcmp(SID,"") == 0) { return false; }
  return true;
  }
};

struct Mqtt {
public: 
  char username[50];
  char password[50];
  bool HA;
};

struct Memory {
public: 
  int task_keepWiFiAlive2=5000;   
  int task_serial_read_task=5000;

  int task_dallas_read=5000;
  int task_updateDimmer=5000;; 
  int task_GetDImmerTemp=5000;

  int task_measure_electricity=5000;
 
  int task_send_mqtt=5000;

  int task_switchDisplay=5000;
  int task_updateDisplay=5000;
  int task_loop=5000; 
};


struct Configmodule {
public: 
  char hostname[16];
  char port[5];
  bool enphase_present=false; 
  bool Fronius_present=false;
  char envoy[5];
  char version[2];
  char token[512]; //correction suite remonté de multinet
};

/// @brief  partie délicate car pas mal d'action sur la variable log_init et donc protection de la variable ( pour éviter les pb mémoire )
struct Logs {
  private:
      char log_init[LOG_MAX_STRING_LENGTH];
      int MaxString = LOG_MAX_STRING_LENGTH * .9 ;

  public:
    bool sct;
    bool sinus;
    bool power;
    bool serial=false; 

  ///setter log_init --> ajout du texte dans la log
public:void Set_log_init(String setter, bool logtime=false) {
        // Vérifier si la longueur de la chaîne ajoutée ne dépasse pas LOG_MAX_STRING_LENGTH
        if ( strlen(setter.c_str()) + strlen(log_init) < static_cast<size_t>(MaxString) )  { 
            if (logtime) { 
              if ( strlen(setter.c_str()) + strlen(log_init) + strlen(loguptime()) < static_cast<size_t>(MaxString))  { 
                strcat(log_init,loguptime()); }
              }
          strcat(log_init,setter.c_str());  
        } else {  
          // Si la taille est trop grande, réinitialiser le log_init
          reset_log_init();
        }     
      }


  ///getter log_init
  public:String Get_log_init() {return log_init; }

  //clean log_init
  public:void clean_log_init() {
      if (strlen(log_init) > (LOG_MAX_STRING_LENGTH - (LOG_MAX_STRING_LENGTH/5)) ) {
      reset_log_init();
      }

      ///si risque de fuite mémoire
      if (strlen(log_init) >(LOG_MAX_STRING_LENGTH - (LOG_MAX_STRING_LENGTH/50)) ) {
      ESP.restart();  
      }
  }

  //reset log_init
  public:void reset_log_init() {
      log_init[0] = '\0';
      strcat(log_init,"197}11}1");
  }

  char *loguptime(bool day=false) {
    static char uptime_stamp[20]; // Vous devrez définir une taille suffisamment grande pour stocker votre temps
      time_t maintenant;
      time(&maintenant);
      if (day) {
        strftime(uptime_stamp, sizeof(uptime_stamp), "%d/%m/%Y %H:%M:%S\t ", localtime(&maintenant));
      } else {
        strftime(uptime_stamp, sizeof(uptime_stamp), "%H:%M:%S\t ", localtime(&maintenant));
      }
    return uptime_stamp;
  }
};

#if DEBUG == true
  #define serial_print(x)  Serial.print (x)
  #define serial_println(x)  Serial.println (x)
#else
  #define serial_print(x)
  #define serial_println(x)
#endif

struct Dallas{
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius = 0.00 ;
  byte security = 0;
  bool detect = false; 
};


//***********************
//****    HA 
//***********************
  #ifndef LIGHT_FIRMWARE
      struct HA
      {


            /* HA */
          private:String name; 
          public:void Set_name(String setter) {name=setter; }
          public:String Get_name() {return name;}

          private:String dev_cla; 
          public:void Set_dev_cla(String setter) {dev_cla=setter; }

          private:String unit_of_meas; 
          public:void Set_unit_of_meas(String setter) {unit_of_meas=setter; }

          private:String stat_cla; 
          public:void Set_stat_cla(String setter) {stat_cla=setter; }

          private:String entity_category; 
          public:void Set_entity_category(String setter) {entity_category=setter; }

          private:String icon; 
          public:void Set_icon(String setter) {icon="\"ic\": \""+ setter +"\", "; }

          bool cmd_t; 

          private:String IPaddress;
          private:String state_topic; 
          private:String stat_t; 
          private:String avty_t;

          
          private:const String node_mac = WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
          private:const String node_id = String("PvRouter-") + node_mac; 
          public:const String topic = "homeassistant/sensor/"+ node_id +"/";
          private:String device_declare() { 
                    String info =         "\"dev\": {"
                    "\"ids\": \""+ node_id + "\","
                    "\"name\": \""+ node_id + "\","
                    "\"sw\": \"PvRouter "+ String(VERSION) +"\","
                    "\"mdl\": \"ESP32 TTGO " + IPaddress + "\","
                    "\"mf\": \"Cyril Poissonnier\","
                    "\"cu\": \"http://"+ IPaddress +"\""
                  "}"; 
                  return info;
                  }
          private:String uniq_id; 
          private:String value_template; 


          public:void discovery(){
            IPaddress =   WiFi.localIP().toString() ;
            String device= "{ \"dev_cla\": \""+dev_cla+"\","
                  "\"unit_of_meas\": \""+unit_of_meas+"\","
                  "\"stat_cla\": \""+stat_cla+"\"," 
                  "\"name\": \""+ name +"-"+ node_mac + "\"," 
                  "\"state_topic\": \""+ topic +"state\","
                  "\"stat_t\": \""+ topic +"state"+name+"\","
                  "\"avty_t\": \""+ topic +"status\","
                  "\"uniq_id\": \""+ node_mac + "-" + name +"\", "
                  "\"value_template\": \"{{ value_json."+name +" }}\", "
                  "\"cmd_t\": \""+ topic +"command\","
                  "\"cmd_tpl\": \"{{ value_json."+name +" }}\", "
                  "\"exp_aft\": \""+ MQTT_INTERVAL +"\", "
                  + icon
                  + device_declare() + 
                "}";
                if (dev_cla =="" ) { dev_cla = name; }

                if (strlen(name.c_str()) != 0 ) {
                client.publish((topic+name+"/config").c_str() , device.c_str() , true); // déclaration autoconf dimmer

                }
                else 
                {
                  client.publish((topic+"config").c_str() , device.c_str() , true); // déclaration autoconf dimmer

                }

                
          }




          public:void send(String value){
            String message = "  { \""+name+"\" : \"" + value.c_str() + "\"  } ";
            client.publish((topic+"state"+name).c_str() , message.c_str(), false); // false for exp_aft in discovery
          }
      
      };
  #endif

struct epoc {
  public:int heure;
  public:int minute;
  public:int seconde;
  public:int jour;
  public:int mois;
  public:int annee;
};

#define TABLEAU_SIZE 7
int tableaudemo[TABLEAU_SIZE] = {180, 3, -150, 4, 150, 5, -180};

#endif
