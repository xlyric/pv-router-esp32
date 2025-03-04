#ifndef ESP32_INFO
#define ESP32_INFO

// ajouté par multinet33

String getShortChipModel() {
  String fullModel = ESP.getChipModel();
  int spaceIndex = fullModel.indexOf(' ');  // Trouve le premier espace
  if (spaceIndex != -1) {
      return fullModel.substring(0, spaceIndex);  // Garde uniquement la première partie
  }
  return fullModel;  // Si pas d'espace, retourne tel quel
}

ESP32Info getESP32Info() {
  ESP32Info info;
  info.chipModel = ESP.getChipModel();
  info.chipModelShort = getShortChipModel();
  info.chipRevision = ESP.getChipRevision();
  info.chipCores = ESP.getChipCores();
  info.boardName = ARDUINO_BOARD;
  info.chipID = ESP.getEfuseMac();
  return info;
}



#endif
