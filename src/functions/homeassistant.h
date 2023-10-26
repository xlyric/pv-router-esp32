#ifndef HOMEASSISTANT
#define HOMEASSISTANT

#include <Arduino.h>

#ifndef LIGHT_FIRMWARE
  #include <PubSubClient.h>


extern HA device_dimmer; 
extern HA device_routeur; 
extern HA device_grid;
extern HA device_inject;
extern HA compteur_inject;
extern HA compteur_grid;
extern HA switch_1;
extern HA temperature_HA;
extern HA power_factor;
extern HA power_vrms;
extern HA power_irms;
extern HA power_apparent;
extern HA enphase_cons_whLifetime;
extern HA enphase_prod_whLifetime;
extern HA enphase_current_power_consumption;
extern HA enphase_current_power_production;
extern HA surplus_routeur;

void init_HA_sensor(){

        device_dimmer.Set_name("Routed");
        device_dimmer.Set_unit_of_meas("W");
        device_dimmer.Set_stat_cla("measurement");
        //device_dimmer.Set_dev_cla("power");
        device_dimmer.Set_dev_cla("power_factor"); // Correct : is using native unit of measurement '%' which is not a valid unit for the device class ('power') it is using
        device_dimmer.Set_icon("mdi:leaf");
        
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

        enphase_cons_whLifetime.Set_name("cons_enphase_cons_whLifetime");
        enphase_cons_whLifetime.Set_unit_of_meas("Wh");
        enphase_cons_whLifetime.Set_stat_cla("total_increasing");
        enphase_cons_whLifetime.Set_dev_cla("energy");      

        enphase_prod_whLifetime.Set_name("cons_enphase_prod_whLifetime");
        enphase_prod_whLifetime.Set_unit_of_meas("Wh");
        enphase_prod_whLifetime.Set_stat_cla("total_increasing");
        enphase_prod_whLifetime.Set_dev_cla("energy");      

        enphase_current_power_consumption.Set_name("enphase_current_power_consumtion");
        enphase_current_power_consumption.Set_unit_of_meas("W");
        enphase_current_power_consumption.Set_stat_cla("measurement");
        enphase_current_power_consumption.Set_dev_cla("power");

        enphase_current_power_production.Set_name("enphase_current_power_production");
        enphase_current_power_production.Set_unit_of_meas("W");
        enphase_current_power_production.Set_stat_cla("measurement");
        enphase_current_power_production.Set_dev_cla("power");

        client.setBufferSize(1024);
        device_routeur.discovery();
        device_dimmer.discovery();
        device_grid.discovery();
        device_inject.discovery();
        compteur_inject.discovery();
        compteur_grid.discovery();
        enphase_cons_whLifetime.discovery();
        enphase_prod_whLifetime.discovery();
        enphase_current_power_consumption.discovery();
        enphase_current_power_production.discovery();
        surplus_routeur.discovery();


        // temperature_HA.discovery();
        // switch_1.discovery(); 

        //power_factor.discovery();
        //power_vrms.discovery();
        //power_irms.discovery();
        //power_apparent.discovery();
        temperature_HA.discovery();
        //switch_1.discovery();



}
#endif

#endif
