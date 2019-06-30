#include <ArduinoJson.h>

void setup() {

    Serial.begin(115200);
    String tinta;
    double nivel_atual = 100;
    
    StaticJsonDocument<256> jsonTinta;

    int nivel_tinta_atual = jsonTinta["tinta"]["nivel_tinta"] = nivel_atual;
    nivel_atual -= 0.42;

    serializeJson(jsonTinta, tinta);
    Serial.println(tinta);
    delay(2000);
}

void loop() {
    
}
