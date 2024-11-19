#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_MLX90614.h>
#include <HX711.h>

#define DT_PIN 26 // Pino OUT do sensor conectado ao pino digital 2 do Arduino
#define SCK_PIN 27 // Pino SCK do sensor conectado ao pino digital 3 do Arduino

const char* ssid = "TCC";
const char* password = "Boiola12";
const char* serverName = "http://192.168.223.208:5050/mensagem/";  // Substitua pelo IP do seu servidor


HX711 scale;
const int sensorP = 34;
float offset = 0;            // Offset inicial para calibração
float calibrationFactor = 1.0; // Fator de calibração para conversão em Pascal



Adafruit_MLX90614 mlx = Adafruit_MLX90614();


void setup() {
  Wire.begin(21, 22); 
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  mlx.begin();
  scale.begin(DT_PIN, SCK_PIN);

  Serial.println("Calibrando... Não aplique pressão ao sensor");
  delay(2000); // Aguardar para estabilização

  // Captura do offset inicial (leitura em zero de pressão)
  scale.set_scale();  // Configura o fator de escala para leitura bruta
  offset = scale.read_average(20); // Leitura média para o offset
  calibrationFactor = 0.5; // Ajuste este valor conforme a calibração
  scale.set_scale(calibrationFactor);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao WiFi");

  pinMode(sensorP, INPUT);
}

void loop() {
  float pressureInPa = (scale.read_average(10) - offset);
  float pressureInMmHg = pressureInPa / 133.322;
  double temp = mlx.readObjectTempC();
  int leitura = analogRead(sensorP);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");


    
    String jsonPayload = "{\"temperatura\": " + String(temp) + ", \"ecg\": " + String(leitura) + ", \"pressao\": "+ String(pressureInMmHg) + "}";
    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      String response = http.getString();
    } else {
      Serial.print("Erro ao enviar mensagem: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("Erro de conexão WiFi");
  }
  Serial.println(leitura);
  delay(100); // Envia a cada 10 segundos
}
