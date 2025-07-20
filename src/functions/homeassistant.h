#ifndef HOMEASSISTANT
#define HOMEASSISTANT

//***********************************
//************* LIBRAIRIES ESP
//***********************************
#include <Arduino.h>
#ifndef LIGHT_FIRMWARE
  #include <PubSubClient.h>
#endif

//***********************************
//************* Variables externes
//***********************************
#ifndef LIGHT_FIRMWARE
  extern HA device_dimmer; 
  extern HA device_dimmer_power; // Ajout RV - pourcentage de puissance du dimmer local
  extern HA device_routeur; 
  extern HA device_routed; // Ajout RV - puissance totale routée : locale + distante
  extern HA device_grid;
  extern HA device_inject;
  extern HA compteur_inject;
  extern HA compteur_grid;
  extern HA switch_1;
  extern HA temperature_HA;
  extern HA power_factor;
  extern HA switch_relay1;
  extern HA switch_relay2;
  extern HA device_dimmer_boost;
  extern HA power_apparent;
  extern HA device_dimmer_alarm_temp; 
#endif

#ifndef LIGHT_FIRMWARE
  //***********************************
  //************* init_HA_sensor
  //***********************************
  void init_HA_sensor() {
    device_dimmer.Set_name("Puissance");
    device_dimmer.Set_unit_of_meas("%");
    device_dimmer.Set_stat_cla("measurement");
    device_dimmer.Set_dev_cla("power_factor"); // Correct : is using native unit of measurement '%' which is not a valid unit for the device class ('power') it is using
    device_dimmer.Set_icon("mdi:percent");

    device_dimmer_power.Set_name("Watt");
    device_dimmer_power.Set_unit_of_meas("W");
    device_dimmer_power.Set_stat_cla("measurement");
    device_dimmer_power.Set_dev_cla("power"); // Correct : is using native unit of measurement '%' which is not a valid unit for the device class ('power') it is using
    device_dimmer_power.Set_icon("mdi:home-lightning-bolt-outline");

    device_routed.Set_name("Routed");
    device_routed.Set_unit_of_meas("W");
    device_routed.Set_stat_cla("measurement");
    device_routed.Set_dev_cla("power"); // Correct : is using native unit of measurement '%' which is not a valid unit for the device class ('power') it is using
    device_routed.Set_icon("mdi:leaf");
    
    device_routeur.Set_name("power");
    device_routeur.Set_unit_of_meas("W");
    device_routeur.Set_stat_cla("measurement");
    device_routeur.Set_dev_cla("power");

    device_grid.Set_name("grid");
    device_grid.Set_unit_of_meas("W");
    device_grid.Set_stat_cla("measurement");
    device_grid.Set_dev_cla("power");

    device_inject.Set_name("inject");
    device_inject.Set_unit_of_meas("W");
    device_inject.Set_stat_cla("measurement");
    device_inject.Set_dev_cla("power");

    compteur_grid.Set_name("grid_Wh");
    compteur_grid.Set_unit_of_meas("Wh");
    compteur_grid.Set_stat_cla("total_increasing");
    compteur_grid.Set_dev_cla("energy");

    compteur_inject.Set_name("inject_Wh");
    compteur_inject.Set_unit_of_meas("Wh");
    compteur_inject.Set_stat_cla("total_increasing");
    compteur_inject.Set_dev_cla("energy");      
            
    temperature_HA.Set_name("Dallas");
    temperature_HA.Set_dev_cla("temperature"); 
    temperature_HA.Set_unit_of_meas("°C");
    temperature_HA.Set_stat_cla("measurement");

    switch_1.Set_name("Switch1");
    switch_1.Set_dev_cla("switch"); 

    power_factor.Set_name("PowerFactor");
    power_factor.Set_unit_of_meas("None");
    power_factor.Set_stat_cla("measurement");
    power_factor.Set_dev_cla("power_factor");

    power_apparent.Set_name("ApparentPower");
    power_apparent.Set_unit_of_meas("VA");
    power_apparent.Set_stat_cla("measurement");
    power_apparent.Set_dev_cla("apparent_power");

      // création des binary_sensor
    device_dimmer_alarm_temp.Set_name("Surchauffe");
    device_dimmer_alarm_temp.Set_dev_cla("enum");
    device_dimmer_alarm_temp.Set_icon("mdi:alert");
    
    switch_relay1.Set_name("Relay1");
    switch_relay1.Set_dev_cla("switch");

    switch_relay2.Set_name("Relay2");
    switch_relay2.Set_dev_cla("switch");

    device_dimmer_boost.Set_name("Boost");
    device_dimmer_boost.Set_dev_cla("switch");
    
    device_routeur.discovery();
    device_routed.discovery();
    device_dimmer.discovery();
    device_dimmer_power.discovery();
    device_grid.discovery();
    device_inject.discovery();
    compteur_inject.discovery();
    compteur_grid.discovery();
    temperature_HA.discovery();
    switch_relay1.discovery();
    switch_relay2.discovery();
    device_dimmer_boost.discovery();
    device_dimmer_alarm_temp.discovery();
    device_dimmer_alarm_temp.send("RAS"); // Initialisation de l'état de l'alarme à RAS
    
  }
#endif // not LIGHT_FIRMWARE

#endif
