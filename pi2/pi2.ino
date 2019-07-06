#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

const int NIVEL_1 = 4;
const int NIVEL_2 = 22;
const int NIVEL_3 = 23;
const int RELE = 16;
const int TIMEZONE = -3;
const char* ssid = "MrVictor42";
const char* password = "bgatahkei42";

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
      
        nivel_atual_tinta = 80;
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
    pinMode(RELE, OUTPUT);    
}

void loop() {

    if ((WiFi.status() == WL_CONNECTED)) { 

        int status_curb = 0;
        HTTPClient httpGet;
     
        httpGet.begin("https://www.jsonstore.io/6ab2d2053ab011dea0384adc74c574ac48fd77f06bcd69b8f7e321fc902fcca8"); 
        int httpCode = httpGet.GET();  
     
        if (httpCode > 0) {
     
            String payload = httpGet.getString();
            StaticJsonDocument<256> json;

            DeserializationError error = deserializeJson(json, payload);
            
            if (error) {
              
                Serial.print(F("deserializeJson() falhou: "));
                Serial.println(error.c_str());
                return;
            }

            status_curb = json["result"]["status_carrinho"].as<int>();

            if(status_curb == 1) {

                digitalWrite(RELE, HIGH);

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

                if(contador_bateria == 2) {

                    httpPost.begin("http://gustavo2795.pythonanywhere.com/monitoramentos/");      
                    httpPost.addHeader("Content-Type", "application/json");  

                    nivel_bateria -=20;
                    monitoring["bateria"] = nivel_bateria;
                    int httpCode = httpPost.POST(dados);   
                 
                    httpPost.end();
                    vTaskDelay(15000);
                    contador_bateria = 0; 
                } else {

                    httpPost.begin("http://gustavo2795.pythonanywhere.com/monitoramentos/");      
                    httpPost.addHeader("Content-Type", "application/json");  

                    int httpCode = httpPost.POST(dados);   
                 
                    httpPost.end();
                    vTaskDelay(15000); 
                }  
            } else {

                digitalWrite(RELE, LOW);

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
