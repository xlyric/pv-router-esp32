#ifndef TASK_FETCH_TIME_NTP
#define TASK_FETCH_TIME_NTP

    
#if NTP_TIME_SYNC_ENABLED == true
    #include <Arduino.h>
    #include <WiFi.h>
    #include <NTPClient.h>
    #include <WiFiUdp.h>
    #include <NTPClient.h>
    #include "../config/enums.h"
    #include <TimeLib.h> 


    extern void reconnectWifiIfNeeded();
    extern DisplayValues gDisplayValues;

    extern NTPClient timeClient;

    void fetchTimeFromNTP(void * parameter){
        for(;;){
            
            if(!WiFi.isConnected()){   /// si pas de connexion Wifi test dans 10 s 
                vTaskDelay(10*1000 / portTICK_PERIOD_MS);
                continue;
            }

            
            unsigned long unix_epoch = timeClient.getEpochTime();
            int Mois, jour; 
            jour   = day(unix_epoch);
            Mois  = month(unix_epoch);


            //detection été /hiver
            if (Mois > 10 || Mois < 3 
            || (Mois == 10 && (jour) > 22) 
            || (Mois == 3 && (jour)<24)){
                //C'est l'hiver
                timeClient.setTimeOffset(NTP_OFFSET_SECONDS*2); 
                }
                else{
                //C'est l'été
                timeClient.setTimeOffset(NTP_OFFSET_SECONDS*1); 
            }

 /*   modifié et reboot tous les lundi à 00:00 pour ne pas avoir des erreurs dans les calculs de puissance
            // reboot de précaution nettoyage mémoire ( dimanche 4:00:00 )
            int uptime = esp_timer_get_time()/ 1000000; 
            if ( uptime > 600000 ) {
                ESP.restart(); 
            }
*/

            serial_println("[NTP] Updating...");

            //String timestring = timeClient.getFormattedTime();
            //short tIndex = timestring.indexOf("T");
           // gDisplayValues.time = timestring.substring(tIndex + 1, timestring.length() -3);
            
            serial_println("[NTP] Done");

            
            // Sleep for a minute before checking again
            vTaskDelay(NTP_UPDATE_INTERVAL_MS / portTICK_PERIOD_MS);
        }
    }

        #endif

#endif
