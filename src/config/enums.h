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

//// NTP 
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_SERVER, NTP_OFFSET_SECONDS, NTP_UPDATE_INTERVAL_MS);

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
  char hostname[16];
  int port;
  char apiKey[64];
  bool UseDomoticz;
  bool UseJeedom;
  int IDX;
  int IDXdallas; 
  char otapassword[64];
  int delta;
  int deltaneg;
  int cosphi;
  int readtime;
  int cycle;
  bool sending;
  bool autonome;
  char dimmer[16];
  bool dimmerlocal;
  float facteur;
  int num_fuse;
  int localfuse;
  int tmax;
  bool mqtt;
  char mqttserver[16];
  int mqttport; 
  int IDXdimmer;
  int resistance;
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
};

struct Configwifi {

  char SID[32];
  char passwd[64];

};

struct Mqtt {
  char username[50];
  char password[50];
  bool HA;
};

struct Memory {
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
  char hostname[16];
  char port[5];
  bool enphase_present=false; 
  bool Fronius_present=false;
  char envoy[5];
  //bool pilote=false; 
  char version[2];
  char token[512]; //correction suite remonté de multinet
};

/// @brief  partie délicate car pas mal d'action sur la variable log_init et donc protection de la variable ( pour éviter les pb mémoire )
struct Logs {
  private:char log_init[LOG_MAX_STRING_LENGTH];
  public:bool sct;
  public:bool sinus;
  public:bool power;
  public:bool serial=false; 

  ///setter log_init --> ajout du texte dans la log
  public:void Set_log_init(String setter, bool logtime=false) {
    // vérification qu'il y ai encore de la taille pour stocker la log 
    if (strlen(log_init) > (LOG_MAX_STRING_LENGTH - (LOG_MAX_STRING_LENGTH/50)) ) {
      reset_log_init();
    }
    if ((strlen(setter.c_str()) + strlen(log_init) > LOG_MAX_STRING_LENGTH)) { return; } // si la taille de la log est trop grande, on ne fait rien )*
    if (logtime) { strcat(log_init,loguptime()); }
    strcat(log_init,setter.c_str()); 
  }


  ///getter log_init
  public:String Get_log_init() {return log_init; }

  //clean log_init
  public:void clean_log_init() {
      if (strlen(log_init) > (LOG_MAX_STRING_LENGTH - (LOG_MAX_STRING_LENGTH/50)) ) {
      reset_log_init();
      }

      ///si risque de fuite mémoire
      if (strlen(log_init) >(LOG_MAX_STRING_LENGTH - (LOG_MAX_STRING_LENGTH/5)) ) {
      //savelogs("-- reboot Suite problème de taille logs -- ");   //--> vu que dans une struc, c'est compliqué à mettre en place
      ESP.restart();  
      }
  }

  //reset log_init
  public:void reset_log_init() {
      log_init[0] = '\0';
      strcat(log_init,"197}11}1");
  }

  char *loguptime() {
    static char uptime_stamp[20]; // Vous devrez définir une taille suffisamment grande pour stocker votre temps
    snprintf(uptime_stamp, sizeof(uptime_stamp), "%s\t", timeClient.getFormattedTime().c_str());
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
          private:const String node_ids = WiFi.macAddress().substring(0,2)+ WiFi.macAddress().substring(4,6)+ WiFi.macAddress().substring(8,10) + WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
          private:const String node_id = String("PvRouter-") + node_mac; 
          private:const String topic = "homeassistant/sensor/"+ node_id +"/";
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


          // private:void online(){
          //   client.publish(String(topic+"status").c_str() , "online", true); // status Online
          // } 

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
                //Serial.println(strlen(name.c_str()));
                if (strlen(name.c_str()) != 0 ) {
                client.publish((topic+name+"/config").c_str() , device.c_str() , true); // déclaration autoconf dimmer
                //Serial.println(topic+name+"/config");
                }
                else 
                {
                  client.publish((topic+"config").c_str() , device.c_str() , true); // déclaration autoconf dimmer
                  //Serial.println(topic+"config");
                }

                // online();
                // send("0");
                
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

#endif
