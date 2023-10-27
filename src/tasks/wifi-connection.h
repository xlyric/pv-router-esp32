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
void keepWiFiAlive(void * parameter){
    for(;;){
        //serial_println(F("Wifi task"));
        if(WiFi.status() == WL_CONNECTED){
                if (AP) { search_wifi_ssid(); }
            vTaskDelay(pdMS_TO_TICKS(30000));
            continue;
        }

        serial_println(F("[WIFI] Connecting"));
        gDisplayValues.currentState = CONNECTING_WIFI;

        WiFi.mode(WIFI_STA);
        WiFi.setHostname(DEVICE_NAME);
        WiFi.begin(configwifi.SID, configwifi.passwd); 
        //else { WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD); }
        

        unsigned long startAttemptTime = millis();

        // Keep looping while we're not connected and haven't reached the timeout
        while (WiFi.status() != WL_CONNECTED && 
                millis() - startAttemptTime < WIFI_TIMEOUT){}

        // Make sure that we're actually connected, otherwise go to deep sleep
        if(WiFi.status() != WL_CONNECTED){
            serial_println(F("[WIFI] FAILED"));
            logging.Set_log_init(loguptime2()); 
            logging.Set_log_init("Wifi disconnected\r\n");
            vTaskDelay(WIFI_RECOVER_TIME_MS / portTICK_PERIOD_MS);
        }

        serial_print(F("[WIFI] Connected: "));
        logging.Set_log_init(loguptime2()); 
        logging.Set_log_init("Wifi reconnected\r\n");
        serial_println(WiFi.localIP());
        serial_print("force du signal:");
        serial_print(WiFi.RSSI());
        serial_println("dBm");
        gDisplayValues.currentState = UP;
        gDisplayValues.IP = String(WiFi.localIP().toString());
        btStop();
    }
}

void keepWiFiAlive2(void * parameter){
    for(;;){
        //serial_println(F("Wifi task"));

                if (AP) { search_wifi_ssid(); 
                }
        
                

            vTaskDelay(pdMS_TO_TICKS(30000));
            //continue;
        }
}

#endif
