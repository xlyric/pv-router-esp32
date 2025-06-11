#ifndef TASK_WIFI_CONNECTION
#define TASK_WIFI_CONNECTION

//***********************************
//************* LIBRAIRIES ESP
//***********************************
#include <Arduino.h>
#include "WiFi.h"

//***********************************
//************* PROGRAMME PVROUTER
//***********************************
#include "../config/enums.h"
#include "../config/config.h"
#include "../functions/WifiFunctions.h"

//***********************************
//************* Variables externes
//***********************************
extern DisplayValues gDisplayValues;
extern Configwifi configwifi; 
extern Logs logging; 
extern Memory task_mem; 

//***********************************
//************* Déclaration de fonctions
//***********************************
extern void goToDeepSleep();

//***********************************
//************* keepWiFiAlive
//***********************************
void keepWiFiAlive(void * parameter) { // NOSONAR
  /**
     * Task: monitor the WiFi connection and keep it alive!
     * 
     * When a WiFi connection is established, this task will check it every 30 seconds 
     * to make sure it's still alive.
     * 
     * If not, a reconnect is attempted. If this fails to finish within the timeout,
     * the ESP32 is send to deep sleep in an attempt to recover from this.
  */
  for(;;) {  
    if(WiFi.status() == WL_CONNECTED) {
      if (AP) { 
        search_wifi_ssid(); 
       }
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
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT) {        
    } // NOSONAR

    // Make sure that we're actually connected, otherwise go to deep sleep
    if(WiFi.status() != WL_CONNECTED) {
      serial_println(F("[WIFI] FAILED"));
             
      logging.Set_log_init(Wifi_disconnected,true);
      vTaskDelay(WIFI_RECOVER_TIME_MS / portTICK_PERIOD_MS);
    }

    serial_print(F(Wifi_reconnected));
         
    logging.Set_log_init(Wifi_reconnected,true);
    serial_println(WiFi.localIP());
    serial_print("force du signal:");
    serial_print(WiFi.RSSI());
    serial_println("dBm");
    gDisplayValues.currentState = DEVICE_STATE::UP;
    gDisplayValues.IP = WiFi.localIP();
    btStop();
  } // for
}

//***********************************
//************* keepWiFiAlive2
/// @brief  task qui permet de rechercher le wifi configuré en cas de passage en mode AP et reboot si trouvé
/// @param parameter 
//***********************************
void keepWiFiAlive2(void * parameter) {
    for(;;) {
      if (AP) { 
        search_wifi_ssid(); 
      }
     
      task_mem.task_keepWiFiAlive2 = uxTaskGetStackHighWaterMark(nullptr);
      vTaskDelay(pdMS_TO_TICKS(30000+(esp_random() % 61) - 30)); 
    } // for

}

#endif
