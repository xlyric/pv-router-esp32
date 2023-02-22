#ifndef HOMEASSISTANT
#define HOMEASSISTANT

#include <Arduino.h>
#include <PubSubClient.h>

extern HA device_dimmer; 
extern HA device_routeur; 
extern HA device_grid;
extern HA device_inject;
extern HA compteur_inject;
extern HA compteur_grid;
extern HA switch_1;
extern HA switch_2;
extern HA temperature_HA;
extern HA power_factor;
extern HA power_vrms;
extern HA power_irms;
extern HA power_apparent;
extern HA device_alarm_temp;


void init_HA_sensor(){

        device_dimmer.Set_name("Puissance Dimmer");
        device_dimmer.Set_object_id("dimmer");
        device_dimmer.Set_unit_of_meas("%");
        device_dimmer.Set_stat_cla("measurement");
        device_dimmer.Set_dev_cla("power_factor"); // fix is using native unit of measurement '%' which is not a valid unit for the device class ('power') it is using
        device_dimmer.Set_icon("mdi:percent");
        device_dimmer.Set_entity_type("sensor");
        
        device_routeur.Set_name("Puissance");
        device_routeur.Set_object_id("power");
        device_routeur.Set_unit_of_meas("W");
        device_routeur.Set_stat_cla("measurement");
        device_routeur.Set_dev_cla("power");
        device_routeur.Set_entity_type("sensor");

        device_grid.Set_name("Soutirage");
        device_grid.Set_object_id("grid");
        device_grid.Set_unit_of_meas("W");
        device_grid.Set_stat_cla("measurement");
        device_grid.Set_dev_cla("power");
        device_grid.Set_entity_type("sensor");


        device_inject.Set_name("Injection");
        device_inject.Set_object_id("inject");
        device_inject.Set_unit_of_meas("W");
        device_inject.Set_stat_cla("measurement");
        device_inject.Set_dev_cla("power");
        device_inject.Set_entity_type("sensor");

        compteur_grid.Set_name("Compteur soutirage");
        compteur_grid.Set_object_id("grid_Wh");
        compteur_grid.Set_unit_of_meas("Wh");
        compteur_grid.Set_stat_cla("total_increasing");
        compteur_grid.Set_dev_cla("energy");
        compteur_grid.Set_entity_type("sensor");


        compteur_inject.Set_name("Compteur injection");
        compteur_inject.Set_object_id("inject_Wh");
        compteur_inject.Set_unit_of_meas("Wh");
        compteur_inject.Set_stat_cla("total_increasing");
        compteur_inject.Set_dev_cla("energy");      
        compteur_inject.Set_entity_type("sensor");
            
        temperature_HA.Set_name("Température");
        temperature_HA.Set_object_id("Dallas");
        temperature_HA.Set_dev_cla("temperature"); 
        temperature_HA.Set_unit_of_meas("°C");
        temperature_HA.Set_stat_cla("measurement");
        temperature_HA.Set_entity_type("sensor");


        switch_1.Set_name("Relais 1");
        switch_1.Set_object_id("Switch1");
        switch_1.Set_entity_type("switch");

        switch_2.Set_name("Relais 2");
        switch_2.Set_object_id("Switch2");
        switch_2.Set_entity_type("switch");

        power_factor.Set_name("Facteur de puissance");
        power_factor.Set_object_id("PowerFactor");
        power_factor.Set_unit_of_meas("");
        power_factor.Set_stat_cla("measurement");
        power_factor.Set_dev_cla("power_factor");
        power_factor.Set_entity_type("sensor");

        power_vrms.Set_name("Tension RMS");
        power_vrms.Set_object_id("Vrms");
        power_vrms.Set_unit_of_meas("V");
        power_vrms.Set_stat_cla("measurement");
        power_vrms.Set_dev_cla("voltage");
        power_vrms.Set_entity_type("sensor");

        power_irms.Set_name("Intensité RMS");
        power_irms.Set_object_id("Irms");
        power_irms.Set_unit_of_meas("A");
        power_irms.Set_stat_cla("measurement");
        power_irms.Set_dev_cla("current");
        power_irms.Set_entity_type("sensor");

        power_apparent.Set_name("Puissance apparente");
        power_apparent.Set_object_id("ApparentPower");
        power_apparent.Set_unit_of_meas("VA");
        power_apparent.Set_stat_cla("measurement");
        power_apparent.Set_dev_cla("apparent_power");
        power_apparent.Set_entity_type("sensor");

        device_alarm_temp.Set_name("Alerte température atteinte");
        device_alarm_temp.Set_object_id("alarm_temp");
        device_alarm_temp.Set_entity_type("binary_sensor");
        device_alarm_temp.Set_stat_cla("problem");





//




//         /// création des button
//         device_save.Set_name("Sauvegarder");
//         device_save.Set_object_id("save");
//         device_save.Set_entity_type("button");
//         device_save.Set_entity_category("config");

//     /// création des number
//   device_starting_pow.Set_name("Puissance de démarrage Dimmer");
//   device_starting_pow.Set_object_id("starting_power");
//   device_starting_pow.Set_entity_type("number");
//   device_starting_pow.Set_entity_category("config");
//   device_starting_pow.Set_entity_valuemin("-100");
//   device_starting_pow.Set_entity_valuemax("500"); // trop? pas assez? TODO : test sans valeur max?
//   device_starting_pow.Set_entity_valuestep("1");

//   device_dimmer_minpow.Set_name("Puissance mini Dimmer");
//   device_dimmer_minpow.Set_object_id("minpow");
//   device_dimmer_minpow.Set_entity_type("number");
//   device_dimmer_minpow.Set_entity_category("config");
//   device_dimmer_minpow.Set_entity_valuemin("0");
//   device_dimmer_minpow.Set_entity_valuemax("100"); // trop? pas assez? TODO : test sans valeur max?
//   device_dimmer_minpow.Set_entity_valuestep("1");

//   device_dimmer_maxpow.Set_name("Puissance maxi Dimmer");
//   device_dimmer_maxpow.Set_object_id("maxpow");
//   device_dimmer_maxpow.Set_entity_type("number");
//   device_dimmer_maxpow.Set_entity_category("config");
//   device_dimmer_maxpow.Set_entity_valuemin("0");
//   device_dimmer_maxpow.Set_entity_valuemax("100"); // trop? pas assez? TODO : test sans valeur max?
//   device_dimmer_maxpow.Set_entity_valuestep("1");

//   device_dimmer_maxtemp.Set_name("Température maxi Dimmer");
//   device_dimmer_maxtemp.Set_object_id("maxtemp");
//   device_dimmer_maxtemp.Set_entity_type("number");
//   device_dimmer_maxtemp.Set_entity_category("config");
//   device_dimmer_maxtemp.Set_entity_valuemin("0");
//   device_dimmer_maxtemp.Set_entity_valuemax("75"); // trop? pas assez? TODO : test sans valeur max?
//   device_dimmer_maxtemp.Set_entity_valuestep("1");
//     /// création des select
//   device_dimmer_child_mode.Set_name("Mode du dimmer");
//   device_dimmer_child_mode.Set_object_id("child_mode");
//   device_dimmer_child_mode.Set_entity_type("select");
//   device_dimmer_child_mode.Set_entity_category("config");
//   device_dimmer_child_mode.Set_entity_option("\"off\",\"delester\",\"equal\"");















        client.setBufferSize(1024);
        device_routeur.discovery();
        device_dimmer.discovery();
        device_grid.discovery();
        device_inject.discovery();
        compteur_inject.discovery();
        compteur_grid.discovery();
        // temperature_HA.discovery();
        switch_1.discovery();
        switch_1.send(String(0));
 
        switch_2.discovery(); 
        switch_2.send(String(0));

        power_factor.discovery();
        power_vrms.discovery();
        power_irms.discovery();
        power_apparent.discovery();




}


#endif