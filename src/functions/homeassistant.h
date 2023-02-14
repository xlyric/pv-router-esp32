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
extern HA power_factor;
extern HA power_vrms;
extern HA power_irms;
extern HA power_apparent;


void init_HA_sensor(){

        device_dimmer.Set_name("dimmer");
        device_dimmer.Set_unit_of_meas("%");
        device_dimmer.Set_stat_cla("measurement");
        //device_dimmer.Set_dev_cla("power");
        device_dimmer.Set_dev_cla("power_factor"); // Correct : is using native unit of measurement '%' which is not a valid unit for the device class ('power') it is using
        device_dimmer.Set_icon("mdi:percent");
        
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

        power_factor.Set_name("PowerFactor");
        power_factor.Set_unit_of_meas("");
        power_factor.Set_stat_cla("measurement");
        power_factor.Set_dev_cla("power_factor");

        power_vrms.Set_name("Vrms");
        power_vrms.Set_unit_of_meas("V");
        power_vrms.Set_stat_cla("measurement");
        power_vrms.Set_dev_cla("voltage");
//        power_vrms.Set_icon("mdi:alpha-v-circle-outline");

        power_irms.Set_name("Irms");
        power_irms.Set_unit_of_meas("A");
        power_irms.Set_stat_cla("measurement");
        power_irms.Set_dev_cla("current");

        power_apparent.Set_name("ApparentPower");
        power_apparent.Set_unit_of_meas("VA");
        power_apparent.Set_stat_cla("measurement");
        power_apparent.Set_dev_cla("apparent_power");

        client.setBufferSize(1024);
        device_routeur.discovery();
        device_dimmer.discovery();
        device_grid.discovery();
        device_inject.discovery();
        compteur_inject.discovery();
        compteur_grid.discovery();

        power_factor.discovery();
        power_vrms.discovery();
        power_irms.discovery();
        power_apparent.discovery();


}


#endif