#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include "config/config.h"
#include "config/enums.h"

extern DisplayValues gDisplayValues;

// WebSocket client instance
WebSocketsClient webSocket;

// WebSocket event handler
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.println("[WSc] Disconnected!");
            break;
        
        case WStype_CONNECTED:
            Serial.println("[WSc] Connected to ws://"+String(config.dimmer)+"/ws");
            // Send a message to the server (optional)
            webSocket.sendTXT("Connected");
            break;
        
        case WStype_TEXT: {
            Serial.printf("[WSc] Received text: %s\n", payload);
            
            // Parse JSON message
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, payload);
            
            if (error) {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.c_str());
                return;
            }
            
            // Extract temperature and Ptotal values
            if (doc.containsKey("temperature") && !dallas.detect ) {
                gDisplayValues.temperature = doc["temperature"].as<float>();
                Serial.printf("Temperature updated: %.2f\n", gDisplayValues.temperature);
            }
            
            if (doc.containsKey("Ptotal")) {
                int local_power =  unified_dimmer.get_power()* config.charge/100;  
                gDisplayValues.puissance_route = local_power + doc["Ptotal"].as<int>();
                Serial.printf("Child power updated: %d\n", doc["Ptotal"].as<int>());
            }
            break;
        }
        
        case WStype_BIN:
            Serial.printf("[WSc] Received binary data: %u bytes\n", length);
            break;
        
        case WStype_ERROR:
            Serial.println("[WSc] Connection error");
            break;
        
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
            // Currently not handling fragmented messages
            break;
    }
}

const char* websocket_server_host = "192.168.1.42"; // IP par défaut du serveur en mode AP
const int websocket_server_port = 80;

// Initialize WebSocket connection
void setupWebSocket() {
    // Server address, port, and URL path
    webSocket.setExtraHeaders("user-agent: ESP32\r\nConnextion: keep-alive\r\n");
    webSocket.begin(websocket_server_host, websocket_server_port, "/ws");
    serial_println("WS : " + String(config.dimmer));
    // Set event handler
    webSocket.onEvent(webSocketEvent);
    
    // Set reconnect interval to 5 seconds
    webSocket.setReconnectInterval(5000);
    
    // Enable heartbeat to keep connection alive
    webSocket.enableHeartbeat(15000, 3000, 2);
    Serial.println("[WSc] WebSocket client initialized");
}

// Function to be called in the main loop to keep the WebSocket connection alive
void handleWebSocket() {
    webSocket.loop();
    
    // If disconnected, try to reconnect
    if (!webSocket.isConnected()) {
        Serial.println("[WSc] Not connected, attempting to reconnect...");
        // The library will handle reconnection based on the interval set
    }
}

void closeWebSocket() {
    webSocket.disconnect();
}

#endif // WEBSOCKET_H
