#ifndef DALLAS_FUNCTIONS
#define DALLAS_FUNCTIONS

//***********************************
//************* PROGRAMME PVROUTEUR
//***********************************
#include "../config/config.h"
#include "../config/enums.h"


//***********************************
//************* Variables externes
//***********************************
extern Dallas dallas ;
extern Logs logging; 

//***********************************
//************* Variables locales
//***********************************
OneWire ds(ONE_WIRE_BUS);
DallasTemperature sensors(&ds);
DeviceAddress insideThermometer;
byte i;
int dallas_error = 0;

//***********************************
//************* dallaspresent()
//************** test si une sonde dallas est présente localement
//***********************************
bool dallaspresent () {
  logging.clean_log_init();

  if ( !ds.search(dallas.addr)) {
    Serial.println(Alerte_Dallas_not_found);
    logging.Set_log_init(Alerte_Dallas_not_found,true);
    Serial.println();
    ds.reset_search();
    delay(250);
  
    return false;
  }
    
  Serial.print("ROM =");
  for(i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(dallas.addr[i], HEX);
  }

  Serial.println();
  
  // the first ROM byte indicates which chip
  switch (dallas.addr[0]) {
    case 0x10:
      Serial.println(DALLAS_TEXT);  // or old DS1820
      dallas.type_s = 1;
      break;
    case 0x28:
      Serial.println(DALLAS_TEXT);
      dallas.type_s = 0;
      break;
    case 0x22:
      Serial.println(DALLAS_TEXT);
      dallas.type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return false;
  } 

  ds.reset();
  ds.select(dallas.addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
    
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
    
  dallas.present = ds.reset();    ///  byte 0 > 1 si present
  ds.select(dallas.addr);    
  ds.write(0xBE);         // Read Scratchpad
    
  Serial.print("  present = ");
  Serial.println(dallas.present, HEX);
        
  logging.Set_log_init(found_Address,true);
  logging.Set_log_init(String(dallas.present, HEX).c_str());
  logging.Set_log_init("\r\n");

  #ifndef LIGHT_FIRMWARE
    if (!discovery_temp) {
      discovery_temp = true;
      temperature_HA.discovery();
    }
  #endif

  dallas.lost = false;

  return true;
}


//***********************************
//************* CheckTemperature
//************* récupération d'une température du 18b20
//***********************************    
float CheckTemperature(String label, byte deviceAddress[12]) { // NOSONAR
  sensors.requestTemperatures(); 
   
  delay(400); // conseillé 375 ms pour une 18b20

  float tempC = sensors.getTempC(deviceAddress);

  if ( (tempC == -127.0) || (tempC == -255.0) ) { 
    //// cas d'une sonde trop longue à préparer les valeurs 
    delay(187); /// attente de 187ms ( temps de réponse de la sonde )
    
    tempC = sensors.getTempC(deviceAddress);
    if ( (tempC == -127.0) || (tempC == -255.0) ) {
      Serial.print("Error getting temperature");
      logging.Set_log_init(Dallas_lost);
       /// si erreur on reprends l'ancienne valeur
       tempC = gDisplayValues.temperature; 
       dallas_error++;
    }
  } 
  else {
    //réduction du retour à 1 décimale 
    tempC = (int(tempC*10))/10.0;
    dallas_error = 0;  
    dallas.lost = false; // on a une valeur donc on est pas perdu
    
    return tempC ; 
  }  

  if (dallas_error > 3) {
    Serial.print("Error getting temperature try to reinit");
    char temp_buffer[128];
    snprintf(temp_buffer, sizeof(temp_buffer), " %s %s times\r\n", Dallas_lost, dallas_error);
    logging.Set_log_init(temp_buffer);
    tempC = gDisplayValues.temperature; 

    /// mise en securité du dimmer local
    unified_dimmer.dimmer_off("Dallas lost");
    unified_dimmer.set_power(0);
    dallas.lost = true; // on est perdu donc on coupe le dimmer
    config.preheat = false;
    
    // on retente une init de la dallas b
    dallas_error = 0;
    dallaspresent();
  }
  
  return tempC; 
}

#endif
