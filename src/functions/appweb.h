#ifndef APPWEB_FUNCTIONS
#define APPWEB_FUNCTIONS

#include "energyFunctions.h"

String configweb; 
extern DisplayValues gDisplayValues;
extern Config config; 
int middleoscillo = 1800;

const char* PARAM_INPUT_1 = "send"; /// paramettre de retour sendmode
const char* PARAM_INPUT_2 = "cycle"; /// paramettre de retour cycle
const char* PARAM_INPUT_3 = "readtime"; /// paramettre de retour readtime
const char* PARAM_INPUT_4 = "cosphi"; /// paramettre de retour cosphi
const char* PARAM_INPUT_save = "save"; /// paramettre de retour cosphi
const char* PARAM_INPUT_dimmer = "dimmer"; /// paramettre de retour cosphi
const char* PARAM_INPUT_server = "server"; /// paramettre de retour server domotique
const char* PARAM_INPUT_IDX = "idx"; /// paramettre de retour idx
const char* PARAM_INPUT_IDXdimmer = "idxdimmer"; /// paramettre de retour idx
const char* PARAM_INPUT_IDXprod = "idxprod"; /// paramettre de retour idx
const char* PARAM_INPUT_port = "port"; /// paramettre de retour port server domotique
const char* PARAM_INPUT_delta = "delta"; /// paramettre retour delta
const char* PARAM_INPUT_deltaneg = "deltaneg"; /// paramettre retour deltaneg
const char* PARAM_INPUT_fuse = "fuse"; /// paramettre retour fusible numérique
const char* PARAM_INPUT_API = "apiKey"; /// paramettre de retour apiKey
const char* PARAM_INPUT_servermode = "servermode"; /// paramettre de retour activation de mode server
const char* PARAM_INPUT_dimmer_power = "POWER"; /// paramettre de retour activation de mode server
const char* PARAM_INPUT_facteur = "facteur"; /// paramettre retour delta
const char* PARAM_INPUT_tmax = "tmax"; /// paramettre retour delta
const char* PARAM_INPUT_mqttserver = "mqttserver"; /// paramettre retour mqttserver
const char* PARAM_INPUT_reset = "reset"; /// paramettre retour mqttserver
const char* PARAM_INPUT_publish = "publish"; /// paramettre retour mqttserver


//***********************************
//** Oscillo mode creation du tableau de mesure pour le graph
//***********************************

String oscilloscope() {

 // int starttime,endtime; 
  int timer = 0; 
  int temp, signe, moyenne; 
  int freqmesure = 40; 
  int sigma = 0;
  String retour = "[[";
  
  front();
  
  delayMicroseconds (config.cosphi*config.readtime); // correction décalage
  while ( timer < ( freqmesure ) )
  {

  
  temp =  analogRead(ADC_INPUT); signe = analogRead(ADC_PORTEUSE);
  moyenne = middleoscillo  + signe/50; 
  sigma += temp;
  //moyenne = moyenne + abs(temp - middle) ;
  /// mode oscillo graph 

  
  retour += String(timer) + "," + String(moyenne) + "," + String(temp) + "],[" ; 
  timer ++ ;
  delayMicroseconds (config.readtime);
  } 
  
  temp =  analogRead(ADC_INPUT); signe = analogRead(ADC_PORTEUSE);
  moyenne = middleoscillo  + signe/50; 
  retour += String(timer) + "," + String(moyenne) + "," + String(temp) + "]]" ;
  middleoscillo = sigma / freqmesure ;

return ( retour ); 
  
}


//***********************************
//************* retour des pages
//***********************************

String getState() {
  String state=STABLE; 
  if (gDisplayValues.watt >= config.delta  ) {   state = GRID; }
  if (gDisplayValues.watt <= config.deltaneg ) {   state = INJECTION; }
  
  state = state + ";" + int(gDisplayValues.watt) + ";" + gDisplayValues.dimmer + ";" + config.delta + ";" + config.deltaneg;
  return String(state);
}

String stringbool(bool mybool){
  String truefalse = "true";
  if (mybool == false ) {truefalse = "";}
  return String(truefalse);
  }
//***********************************
String getSendmode() {
  String sendmode;
  if ( config.sending == 0 ) {   sendmode = "Off"; }
  else {   sendmode = "On"; }
  return String(sendmode);
}
//***********************************
String getServermode(String Servermode) {
  if ( Servermode == "screen" ) {  gDisplayValues.screenstate = !gDisplayValues.screenstate; }
  if ( Servermode == "Jeedom" ) {   config.UseJeedom = !config.UseJeedom;}
  if ( Servermode == "Autonome" ) {   config.autonome = !config.autonome; }
  if ( Servermode == "Dimmer local" ) {   config.dimmerlocal = !config.dimmerlocal; }
  if ( Servermode == "MQTT" ) {   config.mqtt = !config.mqtt; }
  if ( Servermode == "polarité" ) {   config.polarity = !config.polarity; }
  

return String(Servermode);
}
//***********************************
String getSigma() {
   return String(gDisplayValues.watt);
}

//***********************************
String getcosphi() {
  String rawdata ="";
   return String(rawdata) ;
}
//***********************************
String getpuissance() {
  //int tempvalue=15;
   int bestpuissance =1 ;
  return String(bestpuissance*config.facteur) ;
}
//***********************************
String getconfig() {
  configweb = String(config.IDXdimmer) + ";" +  config.num_fuse + ";"  + String(config.IDX) + ";"  +  String(VERSION) +";" + "middle" +";"+ config.delta +";"+config.cycle+";"+config.dimmer+";"+config.cosphi+";"+config.readtime +";"+stringbool(config.UseDomoticz)+";"+stringbool(config.UseJeedom)+";"+stringbool(config.autonome)+";"+config.apiKey+";"+stringbool(config.dimmerlocal)+";"+config.facteur+";"+stringbool(config.mqtt)+";"+config.mqttserver+ ";"  + String(config.Publish)+";"+config.deltaneg+";"+config.resistance+";"+config.polarity+";"+config.ScreenTime+ ";"  + String(config.IDXprod);
  return String(configweb);
}
//***********************************
String getchart() {
  String retour ="" ;
    retour = oscilloscope() ;
      return String(retour);
}
//***********************************

String getdebug() {
  configweb = "";
  configweb += "middle:" + String(middle_debug) + "\r\n" ; 
  //// calcul du cos phi par recherche milieu de demi onde positive 
  int start=0;int end=0;int half=0  ;
    for ( int i=0; i < 72; i ++ )
    {
      if ( porteuse[i] !=0  && start == 0 ) {start = i ;}
      if ( porteuse[i] ==0 && start != 0 && end == 0 ) {end = i ;}
	  configweb += String(i) + "; "+ String(tableau[i]) + "; "+ String(porteuse[i]) + "\r\n" ;
    }
    half = 36 - ( end - start ); 
    configweb += "  cosphi :" + String(half) + "  end  :" + String(end ) +"  start :" + String(start)  ; 


    return String(configweb);
}
//***********************************
String getmemory() {
   String memory = "";
   // memory = String(ESP.getFreeHeap()) + ";" + String(ESP.getHeapFragmentation()) ;
      return String(memory);
}
//***********************************
String getlogs() {
   String logs = "";
      return String(logs);
}

//***********************************
String processor(const String& var){
   Serial.println(var);
   if (var == "SIGMA"){
    return getSigma();
  }
  else if (var == "SENDMODE"){
  
    return getSendmode();
  }
  else if (var == "STATE"){
    
    return getState();
  }  
return getState();
}


//***********************************
//************* Fonction domotique 
//***********************************

void SendToDomotic(String Svalue){
  String baseurl; 
  Serial.print("connecting to mqtt & dimmer");
  Serial.println(config.hostname);
  
 /* if ( config.mqtt == 1 ) {     mqtt(config.IDX,Svalue);  }*/
  Serial.println(baseurl);

  // http.begin(config.hostname,config.port,baseurl);
  //int httpCode = http.GET();
  //Serial.println("closing connection");
  //http.end();
/*
  if ( config.autonome == 1 && change == 1   ) {  baseurl = "/?POWER=" + String(dimmer_power) ; http.begin(config.dimmer,80,baseurl);   int httpCode = http.GET();
    http.end(); 
    if ( config.mqtt == 1 ) { mqtt(config.IDXdimmer, String(dimmer_power));  }
    delay (3000); // delay de transmission réseau dimmer et application de la charge
    */  
    
    //}


 
}

/*
void mqtt(String idx, String value)
{
  String nvalue = "0" ; 
  if ( value != "0" ) { nvalue = "2" ; }
String message = "  { \"idx\" : " + idx +" ,   \"svalue\" : \"" + value + "\",  \"nvalue\" : " + nvalue + "  } ";

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  client.publish("domoticz/in", String(message).c_str(), true);
  
}*/


String injection_type() {
      String state = STABLE; 
      if (gDisplayValues.watt >= config.delta ) {   state = GRID; }
      if (gDisplayValues.watt <= config.deltaneg ) {   state = INJECTION; }
      
      return (state);
}

/*
*  récupération de la température sur le dimmer 
*/
String Dimmer_temp(char* host) {
WiFiClient client;
  
  String url = "/state";
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");
  
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('OK');

  return (line);
   
}
#endif