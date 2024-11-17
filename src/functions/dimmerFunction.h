#ifndef DIMMER_FUNCTIONS
#define DIMMER_FUNCTIONS

#include <WiFi.h>
#include "../config/enums.h"
#include "../config/config.h"
#include "../functions/spiffsFunctions.h"
#include "../functions/Mqtt_http_Functions.h"
#include "../functions/minuteur.h"
#include "unified_dimmer.h"
#include <RBDdimmer.h>
#include "HTTPClient.h"


extern Programme programme; 
extern Dallas dallas ;




    //***********************************
    //************* dimmer
    //***********************************

    int dimmer_security = 60;  // coupe le dimmer toute les X minutes en cas de probleme externe. 
    int dimmer_security_count = 0; 


  //void dimmer_on();

  String dimmergetState(); 





    extern DisplayValues gDisplayValues;
    extern Config config; 
    HTTPClient http;
    extern dimmerLamp dimmer1; 
    extern gestion_puissance unified_dimmer; 


    extern Logs logging;
  #ifndef LIGHT_FIRMWARE
    extern HA device_dimmer; 

  #endif


/*
*   fonction d'envoie de commande au dimmer
*/
/// Modif RV 20240219 
/// Plus besoin !
#define FACTEUR_REGULATION 1 // NOSONAR
/// fonction d'envoie de commande au dimmer distant 

void dimmer_change(char dimmerurl[15], int dimmerIDX, int dimmervalue, int puissance_dispo) { // NOSONAR

  puissance_dispo= int(puissance_dispo*FACTEUR_REGULATION);
  
    /// envoyer la commande avec la valeur gDisplayValues.dimmer vers le dimmer config.dimmer

    #if WIFI_ACTIVE == true

      /// control dimmer 
    if ( strcmp(config.dimmer,"none") != 0 && strcmp(config.dimmer,"") != 0) {
        #ifndef POURCENTAGE
      const String  baseurl = "/?POWER=" + String(dimmervalue) +"&puissance=" + String(puissance_dispo) ; 
        #else
      const String baseurl = "/?POWER=" + String(dimmervalue) ;
        #endif
        // si la puissance routé est de 0 et que le dimmer est à 0 on ne fait rien
        if ( dimmervalue == 0 && gDisplayValues.puissance_route == 0 ) { return ; }
        
        http.begin(dimmerurl,80,baseurl);   
        http.GET();
        http.end(); 
            if (logging.serial){
            Serial.println(POWER_COMMAND + String(dimmervalue));
            }
        if (logging.power) {    
            
            logging.Set_log_init(POWER_COMMAND,true);
            logging.Set_log_init(String(dimmervalue).c_str());
            logging.Set_log_init("% " + String(puissance_dispo) + "W\r\n");
        }
      }

      //// Mqtt send information
      #ifndef LIGHT_FIRMWARE
        if (!AP) {
            if (config.mqtt)  {
            /// A vérifier que c'est necessaire ( envoie double ? )
            /// la valeur 0 doit quand meme être envoyé 
              Mqtt_send(String(dimmerIDX), String(dimmervalue),"","dimmer"); 
              
            }
        }
      #endif
      delay (500); // delay de transmission réseau dimmer et application de la charge } 
      /// 24/01/2023 passage de 1500 à 500ms 

      #endif
}


//***********************************
//************* Fonction aservissement autonome
//***********************************
// fonction de commande de puissance en fonction de la production
void dimmer(){
  gDisplayValues.change = 0; 

  // vérifie que la puissance n'a pas déjà été appliquée
  if (!gDisplayValues.wattIsValid){
    return;
  }
  else {
   gDisplayValues.wattIsValid = false ;
  }
   
   int puissance_dispo = 0; 
   /// pour éviter les erreurs sur le site (inversion delta et deltaneg)
   if (config.delta < config.deltaneg){
   int temp_error_delta; 
   temp_error_delta = config.delta; 
   config.delta = config.deltaneg ; 
   config.deltaneg = temp_error_delta; 
   }

  // 0 -> linky ; 1-> injection  ; 2-> stabilisé

	/// Ajout de la variable delta_cible pour réutilisations multiples , on prends la moyenne des deux valeurs
	int delta_cible = (config.delta+config.deltaneg)/2;
// puissance dispo à envoyer au dimmer distant ou local
  puissance_dispo = -(gDisplayValues.watt-delta_cible);
  //DEBUG_PRINTLN("------- puissance dispo " + String(puissance_dispo) + " -----------");

if ( gDisplayValues.dimmer != 0 && gDisplayValues.watt >= (config.delta) ) {
    gDisplayValues.dimmer += -abs((gDisplayValues.watt-delta_cible)*COMPENSATION/config.charge); 
    gDisplayValues.dimmer += 1 ;
    gDisplayValues.change = 1; 
    } 

    // injection 
    /// si grosse injection on augmente la puissance par extrapolation
  else if ( gDisplayValues.watt <= config.deltaneg ) {   
    
    gDisplayValues.dimmer += abs((delta_cible-gDisplayValues.watt)*COMPENSATION/config.charge) ; 
    gDisplayValues.change = 1 ; 

    } 

    /// test puissance de sécurité mode dimmer distant uniquement

if ( !config.dimmerlocal && gDisplayValues.dimmer >= config.num_fuse) {
    gDisplayValues.dimmer = config.num_fuse; 
    gDisplayValues.change = 1 ; 
    }
  
      /// test puissance de sécurité mode local
  /// Modif RV - j'inverse le sens de la condition, ça consommera moins de CPU

  if ( config.dimmerlocal && gDisplayValues.dimmer >= config.localfuse ) {
    
    /// Modif RV 20240219
    /// Si et seulement si on n'a pas de dimmer enfant, sinon on ne lui routerait aucune puissance !!!!!
    /// Modification du if() qui ne fonctionnait pas à tous les coups

    if ( strcmp(config.dimmer,"") == 0 || strcmp(config.dimmer,"none") == 0 ){ // Si pas de dimmer fils, on bride la puissance 
    gDisplayValues.dimmer = config.localfuse; 
    gDisplayValues.change = 1 ; 
    }
    else if ( gDisplayValues.dimmer >= ( config.localfuse + config.num_fuse ) ) {
      gDisplayValues.dimmer = config.localfuse + config.num_fuse; 
      gDisplayValues.change = 1 ;
    }
  }

    /// valeur négative impossible
  if ( gDisplayValues.dimmer <= 0 && gDisplayValues.dimmer != 0 ) {
     DEBUG_PRINTLN(("-------dimmerFunction gDisplayValues.dimmer %d -----------",__LINE__));
    gDisplayValues.dimmer = 0; 
    gDisplayValues.change = 1 ; 
    }
    
    gDisplayValues.security ++ ;

   //// envoie d'un Zero au dimmer de temps en temps pour des raisons de sécurité
    if ( gDisplayValues.security >= 5 ) { 
      if ( gDisplayValues.dimmer <= 0 ) {
        DEBUG_PRINTLN("------- dimmerFunction gDisplayValues.dimmer " + String(__LINE__) + " -----------");
        gDisplayValues.dimmer = 0; 
        gDisplayValues.change = 1 ; 
        gDisplayValues.security = 0;  
      }
    } 

  ///// si pas de changement on ne fait rien
  if  (gDisplayValues.change)  {

      /// si configuration en dimmer local
    if (config.dimmerlocal) {

        /// COOLER 
        if ( gDisplayValues.dimmer > 10  || programme.run ) { digitalWrite(COOLER, HIGH); } // start COOLER at 10%  }
        else { digitalWrite(COOLER, LOW); }


            
        float dallas_int = gDisplayValues.temperature;
        if (dallas.security) {  // si sécurité alors on envoie la puissance à 0
          float temp_trigger = float(config.tmax) - float(config.tmax*config.trigger/100) ;
          if ( dallas_int < temp_trigger ) {  
          dallas.security = false ; // retrait securité si inférieur au trigger
          ///affichage dans les logs de l état sécurité
          logging.Set_log_init("Security off\r\n");

          gDisplayValues.dimmer = 0 ; 
          //dimmer_on();
          //dimmer1.setPower(gDisplayValues.dimmer);
          DEBUG_PRINTLN("------- dimmerFunction" + String(__LINE__) + " -----------");
          
            unified_dimmer.set_power(gDisplayValues.dimmer);
          // ledcWrite(0, gDisplayValues.dimmer*256/100);  //pas compris pourquoi on mettait 0 ici
          Serial.println("security on -> off");
          
          }
          else {
            unified_dimmer.set_power(0);
            unified_dimmer.dimmer_off();
            programme.run=false;
            ledcWrite(0, 0);
/// Modif RV 20240219 - ajout du test pour ne pas chercher à envoyer une requête vers un fils non configuré
          if ( strcmp(config.dimmer,"") != 0 && strcmp(config.dimmer,"none") != 0 ) {
            dimmer_change( config.dimmer, config.IDXdimmer, gDisplayValues.dimmer,puissance_dispo) ;
          }
        }
      }
      
        else { 

          if ( config.tmax < dallas_int ) {  // si température supérieure à la température max alors on coupe le dimmer local
            dimmer1.setPower(0); 
            unified_dimmer.dimmer_off();
              #ifdef ESP32D1MINI_FIRMWARE
              unified_dimmer.set_power(0);
              #endif
            ledcWrite(0, 0);
/// Modif RV - 20240219
            /// Ca fait aussi un cut de puissance transitoire pour le(s) dimmer(s) distant(s) quand on atteint la tempé
            /// j'ajoute donc ce if() pour qu'on passe à 0 que si on n'a pas d'enfant
            if ( strcmp(config.dimmer,"") == 0 || strcmp(config.dimmer,"none") == 0 ) {
              DEBUG_PRINTLN("------- dimmerFunction gDisplayValues.dimmer " + String(__LINE__) + " -----------");
              gDisplayValues.dimmer = 0 ;
            }
            dallas.security = true ;   /// mise en place sécurité thermique
            Serial.println("security off -> on ");
            logging.Set_log_init("Security On\r\n");
            unified_dimmer.dimmer_off();
          }
          else {
            if (!dallas.security){  // si pas de sécurité alors on envoie la puissance
                // vérification du mode equal si c'est le cas on divise par 2 la puissance et on envoye la puissance dispo aussi au dimmer distant
                if (config.equal)  { gDisplayValues.dimmer = gDisplayValues.dimmer / 2 ; puissance_dispo = puissance_dispo / 2 ; dimmer_change( config.dimmer, config.IDXdimmer, gDisplayValues.dimmer, puissance_dispo ) ;} 
                /// fonctionnement du dimmer local 
                if ( gDisplayValues.dimmer < config.localfuse && !programme.run ) { // si dimmer < localfuse et pas de minuteur alors on envoie la puissance
                  //    dimmer1.setPower(gDisplayValues.dimmer); 
                  unified_dimmer.set_power(gDisplayValues.dimmer);
                  DEBUG_PRINTLN("------- dimmerFunction " + String(__LINE__) + " -----------");     
                  // si en mode équal on ne coupe pas le dimmer distant             
                  if (!config.equal) { dimmer_change( config.dimmer, config.IDXdimmer, 0, puissance_dispo ) ; }  // sinon ça coupe le dimmer distant en mode équal
                 // ledcWrite(0, gDisplayValues.dimmer*256/100);  
                }
                else {
                    //dimmer_on();
/// Modif RV - 20240303
                /// Ajout de ce if() AVANT de modifier la puissance locale ... sinon ça ne sert à rien
                if (unified_dimmer.get_power() < config.localfuse){ // permet d'éviter de trop de donner de puissance au dimmer enfant quand gros soleil d'un coup
                  puissance_dispo = puissance_dispo - ( (config.localfuse - unified_dimmer.get_power())*config.charge/100 ); 
                    DEBUG_PRINTLN("------- dimmerFunction " + String(__LINE__) + " -----------");
                }
                    if (!dallas.lost) { /// Sécurité si pas de perte de la dallas sinon ça clignote
                      //dimmer1.setPower(config.localfuse); 

                      DEBUG_PRINTLN("------- dimmerFunction " + String(__LINE__) + " -----------");
                        
                        unified_dimmer.set_power(gDisplayValues.dimmer);
                   //   ledcWrite(0, config.localfuse*256/100);
                      if ( strcmp(config.dimmer,"") != 0 && strcmp(config.dimmer,"none") != 0 ) { // Modif RV - Autant ne pas envoyer de requête si in n'a pas d'enfant de configuré
                        dimmer_change( config.dimmer, config.IDXdimmer, ( gDisplayValues.dimmer - config.localfuse ), puissance_dispo ) ;
                      }
                   }
                }
            }
          }
        }

    }

    else { dimmer_change( config.dimmer, config.IDXdimmer, gDisplayValues.dimmer, puissance_dispo ) ;  }
 
  
  }
}

//// fonctions for local dimmer

 
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

    /// fonction pour mettre en pause ou allumer le dimmer 
    /*void dimmer_on()
    {

      if (dimmer1.getState()==0) {
        dimmer1.setState(ON);
        delay(50);
        Serial.println("dimmer on");
        }
    }*/


    String dimmergetState() {
      String state; 
      int pow=unified_dimmer.get_power(); 
      state = String(pow) + ";" + String(gDisplayValues.celsius) ; 
      return String(state);
      
    }

//// récupération de la valeur du dimmer distant
int dimmer_getState() {
  int dimmer = 0 ; 
  ///connexion au json distant 
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
    HTTPClient http;  //Declare an object of class HTTPClient
    http.begin("http://" + String(config.dimmer) + "/state");  //Specify request destination
    int httpCode = http.GET();                                                                  //Send the request
    if (httpCode > 0) { //Check the returning code
      String payload = http.getString();   //Get the request response payload

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
    http.end();   //Close connection
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