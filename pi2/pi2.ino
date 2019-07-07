#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <dummy.h>

const uint8_t TRIG_PIN = 22;
const uint8_t ECHO_PIN = 23;
uint32_t print_timer;

const int NIVEL_1 = 32;
const int NIVEL_2 = 35;
const int NIVEL_3 = 34;

const int RELESOFTWARE = 16;
const int RELE1 = 14;
const int RELE2 = 27;
const int RELE3 = 26;
const int RELE4 = 25;
const int RELE5 = 33;

const int TIMEZONE = -3;
const char* ssid = "ASUS_Abraao";
const char* password = "capiroto";

WiFiUDP udp;

static double nivel_tinta_atual = 100.0;
static double nivel_bateria = 100.0;
static int contador_bateria = 0;

struct Date {
    
    int diaDaSemana;
    int dia;
    int mes;
    int ano;
    int hora;
    int minutos;
    int segundos;
};

NTPClient ntpClient(
    
    udp,
    "0.br.pool.ntp.org",
    TIMEZONE * 3600,
    60000
);

Date getDate() {

    char* strDate = (char*)ntpClient.getFormattedDate().c_str();

    Date date;
    sscanf(strDate, "%d-%d-%dT%d:%d:%dZ", &date.ano, &date.mes, &date.dia, &date.hora, &date.minutos, &date.segundos);

    return date;
}

String dataMonitoramento(Date date) {

    String diaDaSemana = "";

    String dia = String(date.dia);
    String mes = String(date.mes);
    String ano = String(date.ano);

    diaDaSemana = dia + "/";
    diaDaSemana += mes + "/";
    diaDaSemana += ano;
    
    return diaDaSemana;                
}

String horaMonitoramento(Date date) {

    String horario = "";

    String hora = String(date.hora);
    String minutos = String(date.minutos);
    String segundos = String(date.segundos);

    horario = hora + ":";
    horario += minutos;

    return horario;
}

void distancia_ultrassom(const uint8_t trig, const uint8_t echo) {
    // Espera 0,5s (500ms) entre medições.
    if (millis() - print_timer > 500) {
      print_timer = millis();
  
      // Pulso de 5V por pelo menos 10us para iniciar medição.
      digitalWrite(TRIG_PIN, HIGH);
      delayMicroseconds(11);
      digitalWrite(TRIG_PIN, LOW);
  
      /* Mede quanto tempo o pino de echo ficou no estado alto, ou seja,
        o tempo de propagação da onda. */
      uint32_t pulse_time = pulseIn(ECHO_PIN, HIGH);
  
      /* A distância entre o sensor ultrassom e o objeto será proporcional a velocidade
        do som no meio e a metade do tempo de propagação. Para o ar na
        temperatura ambiente Vsom = 0,0343 cm/us. */
      //pode ser double distance tbm, mas nao ha a necessidade para este projeto
      int distance = 0.01715 * pulse_time;
      Serial.print(distance);
      Serial.println(" cm");  
    }
}

int nivel_tinta() {

    int nivel_atual_tinta = 0;
//    int nivel_1 = digitalRead(NIVEL_1);
//    int nivel_2 = digitalRead(NIVEL_2);
//    int nivel_3 = digitalRead(NIVEL_3);
      int nivel_1 = 1;
      int nivel_2 = 1;
      int nivel_3 = 1;


    if(nivel_1 == 0 && nivel_2 == 0 && nivel_3 == 0) {
      
        nivel_atual_tinta = 0;
    } else if(nivel_1 == 1 && nivel_2 == 0 && nivel_3 == 0) {
      
        nivel_atual_tinta = 33;
    } else if(nivel_1 == 1 && nivel_2 == 1 && nivel_3 == 0) {
      
        nivel_atual_tinta = 63;
    } else if(nivel_1 == 1 && nivel_2 == 1 && nivel_3 == 1) {
      
        nivel_atual_tinta = 100;
    }

   return nivel_atual_tinta;
}

void setup() {

    Serial.begin(115200);
    
    Serial.println();
    Serial.println();
    Serial.print("Conectando a: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        
        vTaskDelay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi conectado.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    ntpClient.begin();

    while(!ntpClient.update()) {

        Serial.println(".");
        ntpClient.forceUpdate();
        vTaskDelay(500);
    }

    pinMode(NIVEL_1 , INPUT);
    pinMode(NIVEL_2 , INPUT);
    pinMode(NIVEL_3 , INPUT);
    pinMode(RELESOFTWARE, OUTPUT);
    pinMode(RELE1, OUTPUT);
    pinMode(RELE2, OUTPUT);
    pinMode(RELE3, OUTPUT);
    pinMode(RELE4, OUTPUT);
    pinMode(RELE5, OUTPUT);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    digitalWrite(RELE1, HIGH);
    digitalWrite(RELE2, HIGH);
    digitalWrite(RELE3, HIGH);
    digitalWrite(RELE4, HIGH);
    digitalWrite(RELE5, HIGH);
    digitalWrite(TRIG_PIN, HIGH);
}

void loop() {

    if ((WiFi.status() == WL_CONNECTED)) { 

        int status_curb = 0;
        int status_misturador = 0;
        int status_compressor1 = 0;
        int status_compressor2 = 0;
        int ligar = 1;
        
        HTTPClient httpGet;
        HTTPClient httpMisturador;
        HTTPClient httpCompressor1;
        HTTPClient httpCompressor2;
     
        httpGet.begin("https://www.jsonstore.io/6ab2d2053ab011dea0384adc74c574ac48fd77f06bcd69b8f7e321fc902fcca8");
        httpMisturador.begin("https://www.jsonstore.io/56189fbbe9be4c0f3d639eeeb717d800cd5a8b883852947d310edc6926b23762"); 
        httpCompressor1.begin("https://www.jsonstore.io/612eafcc1e18066ef9d8835a9075a4447bd8a2a267c5751fa790d12ca43c86e8");
        httpCompressor2.begin("https://www.jsonstore.io/fc92470219cc59fdc7129e1f03563a1371883a6d658f57babe0a0d74b55620d3");
        
        int httpCode = httpGet.GET();
        int httpMisturadorCode = httpMisturador.GET();
        int httpCompressorCode1 = httpCompressor1.GET();
        int httpCompressorCode2 = httpCompressor2.GET();

        if (httpMisturadorCode > 0) {

            String payload = httpMisturador.getString();
            StaticJsonDocument<256> json;

            DeserializationError error = deserializeJson(json, payload);
            
            if (error) {
              
                Serial.print(F("deserializeJson() falhou MISTURADOR: "));
                Serial.println(error.c_str());
                return;
            }

            status_misturador = json["result"]["misturador"].as<int>();
        }

        if (httpCompressorCode1 > 0) {

            String payload = httpCompressor1.getString();
            StaticJsonDocument<256> json;

            DeserializationError error = deserializeJson(json, payload);
            
            if (error) {
              
                Serial.print(F("deserializeJson() falhou COMPRESSOR1: "));
                Serial.println(error.c_str());
                return;
            }

            status_compressor1 = json["result"]["compressor"].as<int>();
        }

        if (httpCompressorCode2 > 0) {

            String payload = httpCompressor2.getString();
            StaticJsonDocument<256> json;

            DeserializationError error = deserializeJson(json, payload);
            
            if (error) {
              
                Serial.print(F("deserializeJson() falhou COMPRESSOR2: "));
                Serial.println(error.c_str());
                return;
            }

            status_compressor2 = json["result"]["compressor"].as<int>();
        }

        if (status_misturador == 1) {

            digitalWrite(RELE3, LOW);
        } else {

            digitalWrite(RELE3, HIGH);
        }

        if(status_compressor1 == 1) {

            digitalWrite(RELE4, LOW);
            vTaskDelay(2000);
            digitalWrite(RELE1, LOW); 
        } else {

            digitalWrite(RELE1, HIGH);
            vTaskDelay(2000); 
            digitalWrite(RELE4, HIGH);
        }

        if (status_compressor2 == 1) {

            digitalWrite(RELE4, LOW);
            vTaskDelay(2000);
            digitalWrite(RELE2, LOW);   
        } else {

            digitalWrite(RELE2, HIGH);
            vTaskDelay(2000); 
            digitalWrite(RELE4, HIGH);
        }
     
        if (httpCode > 0) {
     
            String payload = httpGet.getString();
            StaticJsonDocument<256> json;

            DeserializationError error = deserializeJson(json, payload);
            
            if (error) {
              
                Serial.print(F("deserializeJson() falhou STATUS CURB: "));
                Serial.println(error.c_str());
                return;
            }

            status_curb = json["result"]["status_carrinho"].as<int>();

            if(ligar == 1) {

                digitalWrite(RELE3, LOW);
                vTaskDelay(2000);
                digitalWrite(RELE1, LOW);
                digitalWrite(RELE2, LOW);
                vTaskDelay(2000);
                digitalWrite(RELE4, LOW);
                vTaskDelay(2000);
                digitalWrite(RELE5, LOW);
                
                String dados;                
                StaticJsonDocument<256> json;
                JsonObject monitoring = json.to<JsonObject>();
                Date date = getDate();   

                monitoring["curbAtivo"] = "1";
                monitoring["tinta"] = nivel_tinta();
                monitoring["bateria"] = nivel_bateria;
                monitoring["data"] = dataMonitoramento(date);
                monitoring["hora"] = horaMonitoramento(date);
                monitoring["latitudeInicial"] = "-15.989832";
                monitoring["logitudeInicial"] = "-48.046540";
                monitoring["latitudeFinal"] = "-15.989814";
                monitoring["logitudeFinal"] = "-48.046495";
                monitoring["status"] = "true";
            
                serializeJson(json, dados);
                Serial.println(dados);
         
                HTTPClient httpPost;
                contador_bateria += 1;

                distancia_ultrassom(TRIG_PIN, ECHO_PIN);

                if(contador_bateria == 2) {

                    httpPost.begin("http://gustavo2795.pythonanywhere.com/monitoramentos/");      
                    httpPost.addHeader("Content-Type", "application/json");  

                    nivel_bateria -=5;
                    monitoring["bateria"] = nivel_bateria;
                    int httpCode = httpPost.POST(dados);   
                 
                    httpPost.end();
                    vTaskDelay(10000);
                    contador_bateria = 0; 
                } else {

                    httpPost.begin("http://gustavo2795.pythonanywhere.com/monitoramentos/");      
                    httpPost.addHeader("Content-Type", "application/json");  

                    int httpCode = httpPost.POST(dados);   
                 
                    httpPost.end();
                    vTaskDelay(10000); 
                }  
            } else {

                digitalWrite(RELE3, HIGH);
                digitalWrite(RELE5, HIGH);
                vTaskDelay(2000);
                digitalWrite(RELE1, HIGH);
                digitalWrite(RELE2, HIGH);
                digitalWrite(RELE4, HIGH);
                
                String dados;                
                StaticJsonDocument<256> json;
                JsonObject monitoring = json.to<JsonObject>();
        
                monitoring["curbAtivo"] = "0";
                monitoring["tinta"] = "";
                monitoring["bateria"] = "";
                monitoring["data"] = "";
                monitoring["hora"] = "";
                monitoring["latitudeInicial"] = "";
                monitoring["logitudeInicial"] = "";
                monitoring["latitudeFinal"] = "";
                monitoring["logitudeFinal"] = "";
                monitoring["status"] = "false";
            
                serializeJson(json, dados);
                Serial.println(dados);
         
                HTTPClient httpPost;    
             
                httpPost.begin("http://gustavo2795.pythonanywhere.com/monitoramentos/");      
                httpPost.addHeader("Content-Type", "application/json");  
             
                int httpCode = httpPost.POST(dados);   
             
                httpPost.end();
                vTaskDelay(2000); 
            }
        }
        else {
            Serial.println("WiFi Desconectada");
        }
     
        httpGet.end(); 
    }
     
    vTaskDelay(1000);
}
