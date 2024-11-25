#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS     1000

#define sensorB 2
const char* ssid = "DALGELA";
const char* password = "veta271284";
const char* serverName = "http://192.168.15.11:5050/mensagem/";  // Substitua pelo IP do seu servidor


float bpm = 0;
float spO2 = 0;
PulseOximeter pox;
uint32_t tsLastReport = 0;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void onBeatDetected()
{
    Serial.println("Beat!");
}
void setup() {
  Wire.begin(21, 22); 
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  mlx.begin();
  
  if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }

    // The default current for the IR LED is 50mA and it could be changed
    //   by uncommenting the following line. Check MAX30100_Registers.h for all the
    //   available options.
    // pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

    // Register a callback for the beat detection
    pox.setOnBeatDetectedCallback(onBeatDetected);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao WiFi");

}

void loop() {
  // Leitura da pressão
  pox.update();
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {

        bpm = pox.getHeartRate();
        spO2 = pox.getSpO2();

        tsLastReport = millis();  
    }

  double temp = mlx.readObjectTempC();
  // Substitui valores NaN por 0
  if (isnan(temp)) temp = 0.0;
  if (isnan(bpm)) bpm = 0;
  if (isnan(spO2)) spO2 = 0;

  // Verifica se a conexão WiFi está ativa
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    // Criando o JSON de forma segura e garantindo que NaN seja convertido para 0
  String jsonPayload = "{\"temperatura\": " + String(temp, 2) + 
                     ", \"bpm\": " + String(bpm) + 
                     ", \"spO2\": " + String(spO2) + "}";

    // Envia o POST
    int httpResponseCode = http.POST(jsonPayload);

    // Verifica o código de resposta
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Resposta do servidor: " + response);
    } else {
      Serial.print("Erro ao enviar mensagem: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("Erro de conexão WiFi");
  }

  // Imprime no monitor serial para debug
  Serial.println("Temperatura: " + String(temp, 2) + " °C");
  Serial.println("batimentos: " + String(bpm) + "bpm");
  Serial.println("spO2: " + String(spO2) + " %");

}