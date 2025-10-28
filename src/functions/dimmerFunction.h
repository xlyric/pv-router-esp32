#ifndef DIMMER_FUNCTIONS
#define DIMMER_FUNCTIONS

//***********************************
//************* Librairies ESP
//***********************************
#include <WiFi.h>
#include <RBDdimmer.h>
#include "HTTPClient.h"

//***********************************
//************* Programme PV ROUTEUR
//***********************************
#include "../config/enums.h"
#include "../config/config.h"
#include "../functions/spiffsFunctions.h"
#ifndef LIGHT_FIRMWARE
  #include "../functions/Mqtt_http_Functions.h"
#endif
#include "../functions/minuteur.h"
#include "unified_dimmer.h"

//***********************************
//************* Variables externes
//***********************************
extern Programme programme; 
extern Programme programme_marche_forcee;
extern Dallas dallas ;
extern DisplayValues gDisplayValues;
extern Config config; 
extern dimmerLamp dimmer1; 
extern gestion_puissance unified_dimmer; 
extern Logs logging;
#ifndef LIGHT_FIRMWARE
  extern HA device_dimmer; 
#endif

//***********************************
//************* Variables locales
//***********************************
int dimmer_security = 60;  // coupe le dimmer toute les X minutes en cas de probleme externe. 
int dimmer_security_count = 0; 
String dimmergetState(); 
HTTPClient http;

/*
*   fonction d'envoie de commande au dimmer
*/
/// Modif RV 20240219 
/// Plus besoin !
#define FACTEUR_REGULATION 1 // NOSONAR

//***********************************
//************* dimmer_change()
//***********************************
void dimmer_change(char dimmerurl[15], int dimmerIDX, int dimmervalue, int puissance_dispo) { // NOSONAR
  puissance_dispo= int(puissance_dispo*FACTEUR_REGULATION);
 /// envoyer la commande avec la valeur gDisplayValues.dimmer vers le dimmer config.dimmer
 #if WIFI_ACTIVE == true
    /// control dimmer 
    if ( strcmp(config.dimmer,"none") != 0 && strcmp(config.dimmer,"") != 0) {
      char baseurl[50]; 
      
      #ifndef POURCENTAGE
        snprintf(baseurl, sizeof(baseurl), "/?POWER=%d&puissance=%d", dimmervalue, puissance_dispo);
      #else
        snprintf(baseurl, sizeof(baseurl), "/?POWER=%d", dimmervalue);
      #endif

      // si la puissance routé est de 0 et que le dimmer est à 0 on ne fait rien
      if ( dimmervalue == 0 && gDisplayValues.puissance_route == 0 ) { 
        return ; 
      }
          
      http.begin(dimmerurl,80,baseurl);   
      http.GET(); // envoie de la commande
      http.end(); // fermeture de la connexion
      if (logging.serial) {
        Serial.println(POWER_COMMAND + String(dimmervalue));
      }
      if (logging.power) {            
        logging.Set_log_init(POWER_COMMAND);
        char buf_int[30];
        snprintf(buf_int, sizeof(buf_int), "%d W\n", puissance_dispo);
        logging.Set_log_init(buf_int);
      }
    }

    delay (500); // delay de transmission réseau dimmer et application de la charge } 
    /// 24/01/2023 passage de 1500 à 500ms 
  #endif // WIFI_ACTIVE

}


//***********************************
//************* dimme()
//************* Fonction aservissement autonome
//***********************************
void dimmer()
{
  gDisplayValues.change = 0;

  if (!gDisplayValues.wattIsValid)
  {
    return;
  }
  else
  {
    gDisplayValues.wattIsValid = false;
  }

  int puissance_dispo = 0;
  /// pour éviter les erreurs sur le site (inversion delta et deltaneg)
  if (config.delta < config.deltaneg)
  {
    int temp_error_delta;
    temp_error_delta = config.delta;
    config.delta = config.deltaneg;
    config.deltaneg = temp_error_delta;
  }

  // 0 -> linky ; 1-> injection  ; 2-> stabilisé
  /// Ajout de la variable delta_cible pour réutilisations multiples
  int delta_cible = (config.delta + config.deltaneg) / 2;
  // puissance dispo
  puissance_dispo = -(gDisplayValues.watt - delta_cible);

  if (gDisplayValues.dimmer != 0 && gDisplayValues.watt >= (config.delta))
  {
    gDisplayValues.dimmer += -abs((gDisplayValues.watt - delta_cible) * COMPENSATION / config.charge);
    gDisplayValues.dimmer += 1;
    gDisplayValues.change = 1;
  }

  // injection
  /// si grosse injection on augmente la puissance par extrapolation
  else if (gDisplayValues.watt <= config.deltaneg)
  {
    gDisplayValues.dimmer += abs((delta_cible - gDisplayValues.watt) * COMPENSATION / config.charge);
    gDisplayValues.change = 1;
  }

  /// test puissance de sécurité mode dimmer distant uniquement
  if (!config.dimmerlocal && gDisplayValues.dimmer >= config.num_fuse)
  {
    gDisplayValues.dimmer = config.num_fuse;
    gDisplayValues.change = 1;
  }

  /// test puissance de sécurité mode local
  /// Modif RV - j'inverse le sens de la condition, ça consommera moins de CPU
  if (config.dimmerlocal && gDisplayValues.dimmer >= config.localfuse)
  {
    /// Modif RV 20240219
    /// Si et seulement si on n'a pas de dimmer enfant, sinon on ne lui routerait aucune puissance !!!!!
    /// Modification du if() qui ne fonctionnait pas à tous les coups
    if (strcmp(config.dimmer, "") == 0 || strcmp(config.dimmer, "none") == 0)
    { // Si pas de dimmer fils, on bride la puissance
      gDisplayValues.dimmer = config.localfuse;
      gDisplayValues.change = 1;
    }
    else if (gDisplayValues.dimmer >= (config.localfuse + config.num_fuse))
    {
      gDisplayValues.dimmer = config.localfuse + config.num_fuse;
      gDisplayValues.change = 1;
    }
  }

  /// valeur négative impossible
  if (gDisplayValues.dimmer <= 0 && gDisplayValues.dimmer != 0)
  {
    DEBUG_PRINTLN(("-------dimmerFunction gDisplayValues.dimmer %d -----------", __LINE__));
    gDisplayValues.dimmer = 0;
    gDisplayValues.change = 1;
  }

  gDisplayValues.security++;
  //// envoie d'un Zero au dimmer de temps en temps pour des raisons de sécurité
  if (gDisplayValues.security >= 5)
  {
    if (gDisplayValues.dimmer <= 0)
    {
      DEBUG_PRINTLN("------- dimmerFunction gDisplayValues.dimmer " + String(__LINE__) + " -----------");
      gDisplayValues.dimmer = 0;
      gDisplayValues.change = 1;
      gDisplayValues.security = 0;
    }
  }

  ///// si pas de changement on ne fait rien et si on ne préchauffe pas
  if (gDisplayValues.change )
  {
    /// si configuration en dimmer local
    if (config.dimmerlocal)
    {
      /// COOLER
      if (gDisplayValues.dimmer > 10 || programme.run || programme_marche_forcee.run)
      {
        digitalWrite(COOLER, HIGH);
      } // start COOLER at 10%  }
      else
      {
        digitalWrite(COOLER, LOW);
      }
      float dallas_int = gDisplayValues.temperature;
      if (dallas.security && dallas.detect && !config.preheat) // si sécurité active et sonde dallas présente et pas de préchauffage
      {
        float temp_trigger = float(config.tmax) - float(config.tmax * config.trigger / 100);
        if (dallas_int < temp_trigger)
        {
          dallas.security = false; // retrait securité si inférieur au trigger
          /// affichage dans les logs de l état sécurité
          logging.Set_log_init(Security_off);

          gDisplayValues.dimmer = 0;
          DEBUG_PRINTLN("------- dimmerFunction" + String(__LINE__) + " -----------");

          unified_dimmer.set_power(gDisplayValues.dimmer);
          Serial.println("security on -> off");
        }
        else
        {
          logging.Set_log_init(Security_on);
          unified_dimmer.set_power(0);
          unified_dimmer.dimmer_off("Security");
          programme.run = false;
          config.preheat = false;
          ledcWrite(0, 0);
          /// Modif RV 20240219 - ajout du test pour ne pas chercher à envoyer une requête vers un fils non configuré
          if (strcmp(config.dimmer, "") != 0 && strcmp(config.dimmer, "none") != 0)
          {
            dimmer_change(config.dimmer, config.IDXdimmer, gDisplayValues.dimmer, puissance_dispo);
          }
        }
      }
      else
      {
        if (config.tmax < dallas_int)
        {
          dimmer1.setPower(0);
          unified_dimmer.dimmer_off("Dallas");
          #ifdef ESP32D1MINI_FIRMWARE
            unified_dimmer.set_power(0);
            config.preheat = false;
          #endif
          ledcWrite(0, 0);
          /// Modif RV - 20240219
          /// Ca fait aussi un cut de puissance transitoire pour le(s) dimmer(s) distant(s) quand on atteint la tempé
          /// j'ajoute donc ce if() pour qu'on passe à 0 que si on n'a pas d'enfant
          if (strcmp(config.dimmer, "") == 0 || strcmp(config.dimmer, "none") == 0)
          {
            DEBUG_PRINTLN("------- dimmerFunction gDisplayValues.dimmer " + String(__LINE__) + " -----------");
            gDisplayValues.dimmer = 0;
          }
          dallas.security = true; /// mise en place sécurité thermique
          Serial.println("security off -> on ");
          logging.Set_log_init(Security_on);
          unified_dimmer.dimmer_off("Dallas 2");
        }
        else
        {
          if (!dallas.security)
          {
            /// fonctionnement du dimmer local

            if (gDisplayValues.dimmer < config.localfuse && !programme.run )
            {
              unified_dimmer.set_power(gDisplayValues.dimmer);
              DEBUG_PRINTLN("------- dimmerFunction " + String(__LINE__) + " -----------");
              dimmer_change(config.dimmer, config.IDXdimmer, 0, puissance_dispo);
            }
            else
            {
              /// Modif RV - 20240303
              /// Ajout de ce if() AVANT de modifier la puissance locale ... sinon ça ne sert à rien
              if (unified_dimmer.get_power() < config.localfuse)
              { // permet d'éviter de trop de donner de puissance au dimmer enfant quand gros soleil d'un coup
                puissance_dispo = puissance_dispo - ((config.localfuse - unified_dimmer.get_power()) * config.charge / 100);
                DEBUG_PRINTLN("------- dimmerFunction " + String(__LINE__) + " -----------");
              }
              if (!dallas.lost)
              { /// Sécurité si pas de perte de la dallas sinon ça clignote
                DEBUG_PRINTLN("------- dimmerFunction " + String(__LINE__) + " -----------");
                unified_dimmer.set_power(gDisplayValues.dimmer);
                //   ledcWrite(0, config.localfuse*256/100);
                if (strcmp(config.dimmer, "") != 0 && strcmp(config.dimmer, "none") != 0)
                { // Modif RV - Autant ne pas envoyer de requête si in n'a pas d'enfant de configuré
                  dimmer_change(config.dimmer, config.IDXdimmer, (gDisplayValues.dimmer - config.localfuse), puissance_dispo);
                }
              }
            }
          }
        }
      }
    }
    else
    {
      dimmer_change(config.dimmer, config.IDXdimmer, gDisplayValues.dimmer, puissance_dispo);
    }
  }
}

//***********************************
//************* dimmer_setup()
//************* Fonction local dimmer
//***********************************
void Dimmer_setup() {
  /// Correction issue full power at start
  pinMode(outputPin, OUTPUT); 
  
  pinMode(COOLER, OUTPUT);
  digitalWrite(COOLER, LOW);

  // configuration dimmer
  dimmer1.begin(NORMAL_MODE, ON); //dimmer initialisation: name.begin(MODE, STATE) 
  dimmer1.setState(ON);
  dimmer1.setPower(0); 
          #ifdef ESP32D1MINI_FIRMWARE
          unified_dimmer.set_power(0);
          #endif
  ledcWrite(0, 0);
  serial_println("Dimmer started...");
}

//***********************************
//************* dimmergetState()
//************* Fonction local dimmer
//***********************************
String dimmergetState() {
  String state; 
  state.reserve(50); 
  int pow=unified_dimmer.get_power(); 
  state = String(pow) + ";" + String(gDisplayValues.celsius) ; 
  return String(state);  
}

//***********************************
//************* dimmer_getState()
//************* récupération de la valeur du dimmer distant
//***********************************
int dimmer_getState() {
  int dimmer = 0 ; 
    
  //Check WiFi connection status
  if (WiFi.status() == WL_CONNECTED) { 
    HTTPClient http;  //Declare an object of class HTTPClient
    http.begin("http://" + String(config.dimmer) + "/state");  //Specify request destination
    int httpCode = http.GET();                                                                  //Send the request
    if (httpCode > 0) { //Check the returning code
      // connexion au json distant 
      String payload = http.getString();   //Get the request response payload
      http.end();   //Close connection
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        /// Modif RV - 20240221
        /// je préfère que cette fonction se poursuivre pour que le calcul que je rajoute plus bas aille à son terme 
        dimmer = 0;
      }
      else {
        /// Modif RV - 20240221
        /// Le fait de récupérer le champ "dimmer" ne récupère que la puissance locale du premier dimmer enfant
        /// il vaut donc mieux aller chercher l'info "Ptotal" plutôt que "dimmer" dans la page http:// - IPDIMMER - /state
        int dimmerWatt = doc["Ptotal"];
        dimmer = (dimmerWatt*100/config.charge);
      }
    }    
  }  

  if (dimmer != 0 ) {  
    /// Modif RV - 20240221
    /// synchronisation de gDisplayValues.dimmer après avoir requêté le dimmer enfant
    if (config.dimmerlocal){
     gDisplayValues.dimmer = unified_dimmer.get_power()+ dimmer;
    }
    else {
      gDisplayValues.dimmer = dimmer;
    }
  }
  else {
    gDisplayValues.dimmer = unified_dimmer.get_power();
  }

  return dimmer ; 
}

#endif
