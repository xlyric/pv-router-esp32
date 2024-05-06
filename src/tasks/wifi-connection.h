#ifndef TASK_WIFI_CONNECTION
#define TASK_WIFI_CONNECTION


#include <Arduino.h>
#include "WiFi.h"
#include "../config/enums.h"
#include "../config/config.h"
#include "../functions/WifiFunctions.h"




extern DisplayValues gDisplayValues;
extern void goToDeepSleep();
extern Configwifi configwifi; 
extern Logs logging; 
/**
 * Task: monitor the WiFi connection and keep it alive!
 * 
 * When a WiFi connection is established, this task will check it every 30 seconds 
 * to make sure it's still alive.
 * 
 * If not, a reconnect is attempted. If this fails to finish within the timeout,
 * the ESP32 is send to deep sleep in an attempt to recover from this.
 */
void keepWiFiAlive(void * parameter){ // NOSONAR
    for(;;){ 
        if(WiFi.status() == WL_CONNECTED){
                if (AP) { search_wifi_ssid(); }
            vTaskDelay(pdMS_TO_TICKS(30000));
            continue;
        }

        serial_println(F("[WIFI] Connecting"));
        gDisplayValues.currentState = DEVICE_STATE::CONNECTING_WIFI;

        WiFi.mode(WIFI_STA);
        WiFi.setHostname(DEVICE_NAME);
        WiFi.begin(configwifi.SID, configwifi.passwd); 

        

        unsigned long startAttemptTime = millis();

        // Keep looping while we're not connected and haven't reached the timeout
        while (WiFi.status() != WL_CONNECTED && 
                millis() - startAttemptTime < WIFI_TIMEOUT){} // NOSONAR

        // Make sure that we're actually connected, otherwise go to deep sleep
        if(WiFi.status() != WL_CONNECTED){
            serial_println(F("[WIFI] FAILED"));
             
            logging.Set_log_init("Wifi disconnected\r\n",true);
            vTaskDelay(WIFI_RECOVER_TIME_MS / portTICK_PERIOD_MS);
        }

        serial_print(F("[WIFI] Connected: "));
         
        logging.Set_log_init("Wifi reconnected\r\n",true);
        serial_println(WiFi.localIP());
        serial_print("force du signal:");
        serial_print(WiFi.RSSI());
        serial_println("dBm");
        gDisplayValues.currentState = DEVICE_STATE::UP;
        gDisplayValues.IP = String(WiFi.localIP().toString());
        btStop();
    }
}

extern Memory task_mem; 

/// @brief  task qui permet de rechercher le wifi configuré en cas de passage en mode AP et reboot si trouvé
/// @param parameter 
void keepWiFiAlive2(void * parameter){
    for(;;){


                if (AP) { search_wifi_ssid(); 
                }
        
                
            task_mem.task_keepWiFiAlive2 = uxTaskGetStackHighWaterMark(NULL);
            vTaskDelay(pdMS_TO_TICKS(30000));

        }
}

#endif
