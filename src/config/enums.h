#ifndef ENUMS
#define ENUMS

#include <Arduino.h>
#ifndef LIGHT_FIRMWARE
  #include <PubSubClient.h>
  extern PubSubClient client;
#endif

#include <Preferences.h> 

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
  String temperature;
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
  String pvname = "PV ROUTER " + WiFi.macAddress().substring(12,14) + WiFi.macAddress().substring(15,17);
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
  char username[50];
  char password[50];
  bool HA;
};

struct Configmodule {
  char hostname[16];
  char port[5];
  bool enphase_present=false; 
  bool Fronius_present=false;
  char envoy[5];
  //bool pilote=false; 
  char version[2];
  char token[425];
};

struct Logs {
  String init;
  String start;
  bool sct;
  bool sinus;
  bool power;
  bool serial=false; 
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

          
          private:String node_mac = WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
          private:String node_ids = WiFi.macAddress().substring(0,2)+ WiFi.macAddress().substring(4,6)+ WiFi.macAddress().substring(8,10) + WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
          private:String node_id = String("PvRouter-") + node_mac; 
          private:String topic = "homeassistant/sensor/"+ node_id +"/";
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
                client.publish((topic+name+"/config").c_str() , device.c_str() , true); // déclaration autoconf dimmer
                //Serial.println(device.c_str());
                Serial.println(name + "HA discovery");
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
