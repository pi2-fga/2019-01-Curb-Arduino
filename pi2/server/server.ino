#include <ESPAsyncWebServer.h>
#include<WiFi.h>

AsyncWebServer server(80);

const char* ssid = "MrVictor";
const char* password = "2334445555";

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
    server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Parque dos principes");
    });       
    server.begin();
}

void loop() {
    
}   
