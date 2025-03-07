#ifndef TASK_HOME_ASSISTANT
#define TASK_HOME_ASSISTANT

#if HA_ENABLED == true
  //***********************************
  //************* LIBRAIRIES ESP
  //***********************************
  #include <Arduino.h>
  #include <WiFiClientSecure.h>
  #include <WiFiClientSecure.h>
  #include <MQTTClient.h>

  //***********************************
  //************* PROGRAMME PVROUTER
  //***********************************
  #include "../config/config.h"

  //***********************************
  //************* Variables externes
  //***********************************
  extern unsigned short measurements[];

  //***********************************
  //************* Variables locales
  //***********************************
  WiFiClientSecure HA_net;
  // @brief  HA_mqtt(512); pas plus sinon bug Socket error on client PvRouter-xxxx, disconnecting.
  MQTTClient HA_mqtt(512);
  const char* PROGMEM HA_discovery_msg = R"({
    "name": ")" DEVICE_NAME R"(",
    "device_class": "power",
    "unit_of_measurement": "W",
    "icon": "mdi:transmission-tower",
    "state_topic": "homeassistant/sensor/)" DEVICE_NAME R"(/state",
    "value_template": "{{ value_json.power}}",
    "device": {
      "name": ")" DEVICE_NAME R"(",
      "sw_version": "2.0",
      "model": "HW V2",
      "manufacturer": "Cyril Poissonnier",
      "identifiers": [")" DEVICE_NAME R"("]
    }
  })";

  //***********************************
  //************* keepHAConnectionAlive
  //***********************************  
  void keepHAConnectionAlive(void * parameter) {
    /**
     * Established a connection to Home Assistant MQTT broker.
     * 
     * This task should run continously. It will check if an
     * MQTT connection is active and if so, will sleep for 1
     * minute. If not, a new connection will be established.
    */    
    for(;;) {
      if(!WiFi.isConnected()) { 
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        continue;
      }

      // When we are connected, loop the MQTT client and sleep for 0,5s
      if(HA_mqtt.connected()){
        HA_mqtt.loop();
        vTaskDelay(250 / portTICK_PERIOD_MS);
        continue;
      }

      serial_println(F("[MQTT] Connecting to HA..."));
      HA_mqtt.begin(HA_ADDRESS, HA_PORT, HA_net);

      long startAttemptTime = millis();

      while (!HA_mqtt.connect(DEVICE_NAME, HA_USER, HA_PASSWORD) && millis() - startAttemptTime < MQTT_CONNECT_TIMEOUT) {
        vTaskDelay(MQTT_CONNECT_DELAY / portTICK_PERIOD_MS);
      }

      if (!HA_mqtt.connected()) {
        serial_println(F("[MQTT] HA connection failed. Waiting 30s.."));
        vTaskDelay(30005 / portTICK_PERIOD_MS);
      }

      serial_println(F("[MQTT] HA Connected!"));
    }
  }


  //***********************************
  //************* HADiscovery
  //***********************************      
  void HADiscovery(void * parameter) {
    /**
     * TASK: Every 15 minutes we send Home Assistant a discovery message
     *       so that the energy monitor shows up in the device registry.
     */    
    for(;;) {
      if(!HA_mqtt.connected()) {
        serial_println("[MQTT] HA: no MQTT connection.");
        vTaskDelay(30 * 1000 / portTICK_PERIOD_MS);
        continue;
      }

      serial_println("[MQTT] HA sending auto discovery");
      HA_mqtt.publish("homeassistant/sensor/" DEVICE_NAME "/config", HA_discovery_msg);
      vTaskDelay(15 * 60 * 1000 / portTICK_PERIOD_MS);
    }
  }

  //***********************************
  //************* sendEnergyToHA
  //*********************************** 
    void sendEnergyToHA(void * parameter) {
      if(!HA_mqtt.connected()){
        serial_println("[MQTT] Can't send to HA without MQTT. Abort.");
        vTaskDelete(nullptr);
      }

      char msg[30];
      strcpy(msg, "{\"power\":");
      strcat(msg, String(measurements[LOCAL_MEASUREMENTS-1]).c_str());
      strcat(msg, "}");

      serial_print("[MQTT] HA publish: ");
      serial_println(msg);

      HA_mqtt.publish("homeassistant/sensor/" DEVICE_NAME "/state", msg);

      // Task is done!
      vTaskDelete(nullptr);
    }
  #endif
#endif
