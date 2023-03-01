#ifndef ENUMS
#define ENUMS

#include <Arduino.h>
#include <PubSubClient.h>
// #include "WiFi.h"
// #include "functions/WifiFunctions.h"
extern PubSubClient client;
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
  String temperature;
  int Fronius_conso; 
  int Fronius_prod; 
  int Fronius_totalconso;
  float celsius;
  int page=1; 
};

struct Config {
  char hostname[16];
  int port;
  char apiKey[64];
  bool UseDomoticz;
  bool UseJeedom;
  int IDX;
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
  float voltage; 
  float offset; 
  bool flip;
  int relayon; 
  int relayoff;
  bool restart;
};

struct Configwifi {
  char SID[32];
  char passwd[64];
};

struct Mqtt {
  char username[50];
  char password[50];
  bool HA;
  bool JEEDOM;
  bool DOMOTICZ;
  bool HTTP;
};

struct Configmodule {
  char hostname[16];
  char port[5];
  bool enphase_present; 
  bool Fronius_present;
  char envoy[5];
  bool pilote; 
};

struct Logs {
  String init;
  String start;
  bool sct;
  bool sinus;
  bool power;
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
//****    MQTT 
//***********************
struct MQTT
{
    private:int MQTT_INTERVAL = 60;

      /* MQTT */
    private:String name; 
    public:void Set_name(String setter) {name=setter; }

    private:String object_id; 
    public:void Set_object_id(String setter) {object_id=setter; }

    private:String dev_cla; 
    public:void Set_dev_cla(String setter) {dev_cla=setter; }

    private:String unit_of_meas; 
    public:void Set_unit_of_meas(String setter) {unit_of_meas=setter; }

    private:String stat_cla; 
    public:void Set_stat_cla(String setter) {stat_cla=setter; }

    private:String entity_category; 
    public:void Set_entity_category(String setter) {entity_category=setter; }
    
    private:String entity_type; 
    public:void Set_entity_type(String setter) {entity_type=setter; }

    private:String icon; 
    public:void Set_icon(String setter) {icon="\"ic\": \""+ setter +"\", "; }

    private:String min; 
    public:void Set_entity_valuemin(String setter) {min=setter; }

    private:String max; 
    public:void Set_entity_valuemax(String setter) {max=setter; }

    private:String step; 
    public:void Set_entity_valuestep(String setter) {step=setter; }

    private:String entity_option; 
    public:void Set_entity_option(String setter) {entity_option=setter; }

    private:bool retain_flag; 
    public:void Set_retain_flag(bool setter) {retain_flag=setter; }

    private:String retain; 
    public:void Set_retain(bool setter) {
      if (setter) {retain="\"ret\":true,"; }
    }

    private:String expire_after; 
    public:void Set_expire_after(bool setter) {
      if (setter) {expire_after="\"exp_aft\": \""+ String(MQTT_INTERVAL) +"\", "; }
    }

    private:String HA_sensor_type() {
      String topic = "homeassistant/"+ entity_type +"/"+ node_id +"/";
      String topic_Xlyric = "Xlyric/"+ node_id +"/";
      String info;
      if (entity_type == "sensor") {
              info =         "\"dev_cla\": \""+dev_cla+"\","
            "\"unit_of_meas\": \""+unit_of_meas+"\","
            "\"stat_cla\": \""+stat_cla+"\"," 
            "\"value_template\": \"{{ value_json."+ object_id +" }}\","; 
      }
      else if (entity_type == "switch") { 
              info =         "\"val_tpl\": \"{{ value_json."+ object_id +" }}\","
            "\"pl\":  \"{{ value_json."+ object_id +" }}\","
            "\"pl_on\": \"{ \\\""+object_id+"\\\" : \\\"1\\\"  } \","
            "\"pl_off\": \"{ \\\""+object_id+"\\\" : \\\"0\\\"  } \","
            "\"stat_on\":1,"
            "\"stat_off\":0,"
            "\"cmd_t\": \""+ topic_Xlyric + entity_type + "/command\",";
      } 
      // else if (entity_type == "number") { 
      //         info =         "\"val_tpl\": \"{{ value_json."+ object_id +" }}\","
      //       "\"cmd_t\": \""+ topic + "command\","
      //       "\"cmd_tpl\": \"{ \\\""+object_id+"\\\" : {{ value }} } \"," 
      //       "\"entity_category\": \""+ entity_category + "\","
      //       "\"max\": \""+max+"\","
      //       "\"min\": \""+min+"\","
      //       "\"step\": \""+step+"\",";
      // } 
      // else if (entity_type == "select") { 
      //         info =         "\"val_tpl\": \"{{ value_json."+ object_id +" }}\","
      //       "\"cmd_t\": \""+ topic + "command\","
      //       "\"cmd_tpl\": \"{ \\\""+object_id+"\\\" : \\\"{{ value }}\\\" } \","
      //       "\"entity_category\": \""+ entity_category + "\","
			//       "\"options\": ["+ entity_option + "],";
      // } 
      else if (entity_type == "binary_sensor") { 
              info =         "\"dev_cla\": \""+dev_cla+"\","
            "\"pl_on\":\"true\","
            "\"pl_off\":\"false\","
            "\"val_tpl\": \"{{ value_json."+ object_id +" }}\",";
      }
      // else if (entity_type == "button") { 
      //         info =            "\"entity_category\": \""+ entity_category + "\","
      //       "\"cmd_t\": \""+ topic + "command\","
      //       "\"pl_prs\": \"{ \\\""+object_id+"\\\" : \\\"1\\\"  } \",";
      // }
      return info;
    }

    private:String node_mac = WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
    private:String node_id = String("PvRouter-") + node_mac; 
    private:String HA_device_declare() { 
              String IPaddress = String(WiFi.localIP().toString());
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


    public:void HA_discovery(){
      String topic = "homeassistant/"+ entity_type +"/"+ node_id +"/";
      String topic_Xlyric = "Xlyric/"+ node_id +"/";

      String device= "{\"name\": \""+ name + "\"," 
            "\"obj_id\": \"PvRouter-"+ object_id +"-"+ node_mac + "\"," 
            "\"uniq_id\": \""+ node_mac + "-" + object_id +"\","
            "\"stat_t\": \""+ topic_Xlyric + "sensors/" + object_id +"/state\"," 
            "\"avty_t\": \""+ topic_Xlyric + "status\","
            + HA_sensor_type()
            + icon
            + retain
            + expire_after
            + HA_device_declare() + 
            "}";
      client.publish((topic+object_id+"/config").c_str() , device.c_str(), true); // déclaration autoconf PvRouter
      Serial.println(device.c_str());

    }

    // public:void send(String value){
    //   String topic = "homeassistant/"+ entity_type +"/"+ node_id +"/";
    //   String message = "  { \""+object_id+"\" : \"" + value.c_str() + "\"  } ";
    //   client.publish((topic + object_id + "/state").c_str() , message.c_str(), retain_flag);
    // } 

    public:void send(String value){
    String topic = "Xlyric/"+ node_id +"/sensors/";
    String message = "  { \""+object_id+"\" : \"" + value.c_str() + "\"  } ";
    client.publish((topic + object_id + "/state").c_str() , message.c_str(), retain_flag);
  } 
};


#endif