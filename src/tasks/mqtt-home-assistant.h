#ifndef TASK_HOME_ASSISTANT
#define TASK_HOME_ASSISTANT

#include <Arduino.h>
#include <WiFiClientSecure.h>



#if HA_ENABLED == true


    #include <WiFiClientSecure.h>
    #include <MQTTClient.h>
    #include "../config/config.h"

    WiFiClientSecure HA_net;
    /// @brief  HA_mqtt(512); pas plus sinon bug Socket error on client PvRouter-xxxx, disconnecting.
    MQTTClient HA_mqtt(512);

    extern unsigned short measurements[];

    const char* PROGMEM HA_discovery_msg = "{"
            "\"name\":\"" DEVICE_NAME "\","
            "\"device_class\":\"power\","
            "\"unit_of_measurement\":\"W\","
            "\"icon\":\"mdi:transmission-tower\","
            "\"state_topic\":\"homeassistant/sensor/" DEVICE_NAME "/state\","
            "\"value_template\":\"{{ value_json.power}}\","
            "\"device\": {"
                "\"name\":\"" DEVICE_NAME "\","
                "\"sw_version\":\"2.0\","
                "\"model\":\"HW V2\","
                "\"manufacturer\":\"Cyril Poissonnier\","
                "\"identifiers\":[\"" DEVICE_NAME "\"]"
            "}"
        "}";

    /**
     * Established a connection to Home Assistant MQTT broker.
     * 
     * This task should run continously. It will check if an
     * MQTT connection is active and if so, will sleep for 1
     * minute. If not, a new connection will be established.
     */
    void keepHAConnectionAlive(void * parameter){
        for(;;){

            if(!WiFi.isConnected()){
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
        
            while (!HA_mqtt.connect(DEVICE_NAME, HA_USER, HA_PASSWORD) &&
                    millis() - startAttemptTime < MQTT_CONNECT_TIMEOUT)
            {
                vTaskDelay(MQTT_CONNECT_DELAY / portTICK_PERIOD_MS);
            }

            if(!HA_mqtt.connected()){
                serial_println(F("[MQTT] HA connection failed. Waiting 30s.."));
                vTaskDelay(30000 / portTICK_PERIOD_MS);
            }

            serial_println(F("[MQTT] HA Connected!"));
        }
    }

    /**
     * TASK: Every 15 minutes we send Home Assistant a discovery message
     *       so that the energy monitor shows up in the device registry.
     */
    void HADiscovery(void * parameter){
        for(;;){
            if(!HA_mqtt.connected()){
                serial_println("[MQTT] HA: no MQTT connection.");
                vTaskDelay(30 * 1000 / portTICK_PERIOD_MS);
                continue;
            }

            serial_println("[MQTT] HA sending auto discovery");
            HA_mqtt.publish("homeassistant/sensor/" DEVICE_NAME "/config", HA_discovery_msg);
            vTaskDelay(15 * 60 * 1000 / portTICK_PERIOD_MS);
        }
    }

    void sendEnergyToHA(void * parameter){
        if(!HA_mqtt.connected()){
        serial_println("[MQTT] Can't send to HA without MQTT. Abort.");
        vTaskDelete(NULL);
        }

        char msg[30];
        strcpy(msg, "{\"power\":");
            strcat(msg, String(measurements[LOCAL_MEASUREMENTS-1]).c_str());
        strcat(msg, "}");

        serial_print("[MQTT] HA publish: ");
        serial_println(msg);

        HA_mqtt.publish("homeassistant/sensor/" DEVICE_NAME "/state", msg);

        // Task is done!
        vTaskDelete(NULL);
    }

        #endif

#endif
