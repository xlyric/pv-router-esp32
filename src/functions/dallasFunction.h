#ifndef DALLAS_FUNCTIONS
#define DALLAS_FUNCTIONS

#include "../config/config.h"
#include "../config/enums.h"


    //***********************************
    //************* Test de la présence d'une 18b20 
    //***********************************
    
extern Dallas dallas ;
extern Logs logging; 
OneWire ds(ONE_WIRE_BUS);
DallasTemperature sensors(&ds);
DeviceAddress insideThermometer;

byte i;

bool dallaspresent () {
  for (int i = 0; i < deviceCount; i++) {
    if (!ds.search(addr[i])) {
      logging.Set_log_init("Unable to find temperature sensors address \r\n",true);
      ds.reset_search();
      delay(350);
      return false;
      }
  }
  for (int a = 0; a < deviceCount; a++) {
    String address = "";
    Serial.print("ROM =");
      for (uint8_t i = 0; i < 8; i++) {
        if (addr[a][i] < 16) address += "0";
        address += String(addr[a][i], HEX);
        Serial.write(' ');
        Serial.print(addr[a][i], HEX);
      }
    devAddrNames[a] = address;
    Serial.println();
      if (strcmp(address.c_str(), config.DALLAS) == 0) {
        dallas.dallas_maitre = a;
        logging.Set_log_init("MAIN " );
      }

    logging.Set_log_init("Dallas sensor " );
    logging.Set_log_init(String(a).c_str()); 
    logging.Set_log_init(" found. Address : " );
    logging.Set_log_init(String(address).c_str()); 
    logging.Set_log_init("\r\n");

    delay(250);



    ds.reset();
    ds.select(addr[a]);

    ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
    delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
    dallas.present = ds.reset();    ///  byte 0 > 1 si present
    ds.select(addr[a]);    
    ds.write(0xBE);         // Read Scratchpad
   }
   return true;
  }

    //***********************************
    //************* récupération d'une température du 18b20
    //***********************************

float CheckTemperature(String label, byte deviceAddress[12]){ // NOSONAR


  float tempC = sensors.getTempC(deviceAddress);

  if ( (tempC == -127.00) || (tempC == -255.00) ) {
    delay(250);
    //// cas d'une sonde trop longue à préparer les valeurs 
     /// attente de 187ms ( temps de réponse de la sonde )
    tempC = sensors.getTempC(deviceAddress);
  }  
  return tempC; 
}

#endif