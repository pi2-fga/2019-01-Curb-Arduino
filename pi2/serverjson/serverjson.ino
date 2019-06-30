#include <ESPAsyncWebServer.h>
#include<WiFi.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include "AsyncJson.h"

AsyncWebServer server(80);

const char* ssid = "MrVictor42";
const char* password = "bgatahkei42";
static double nivel_atual = 100;

void setup() {

    Serial.begin(115200);
    
    Serial.println();
    Serial.println();
    Serial.print("Conectando a: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi conectado.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());    

    server.begin();

    server.on("/dados", HTTP_GET, [](AsyncWebServerRequest *request){
        String dados;
        int contador = 0;
        
        StaticJsonDocument<256> json;

        int nivel_tinta_atual = json["tinta"]["nivel_tinta"] = nivel_atual;
        long int latitude = json["gps"]["latitude"] = 424242;
        nivel_atual -= 0.42;
    
        serializeJson(json, dados);
        request->send(200, "application/json", dados);
    });        
}

void loop() {
  
}
