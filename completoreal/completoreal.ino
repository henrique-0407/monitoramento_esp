#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_MLX90614.h>
#include <HX711.h>

#define DT_PIN 26 // Pino OUT do sensor conectado ao pino digital 2 do Arduino
#define SCK_PIN 27 // Pino SCK do sensor conectado ao pino digital 3 do Arduino

const char* ssid = "DALGELA";
const char* password = "veta271284";
const char* serverName = "http://192.168.15.11:5050/mensagem/";  // Substitua pelo IP do seu servidor

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
  // Leitura da pressão
  float pressureInPa = (scale.read_average(10) - offset);
  float pressureInMmHg = pressureInPa / 133.322;
  
  // Leitura da temperatura
  double temp = mlx.readObjectTempC();
  
  // Leitura do sensor de ECG
  int leitura = analogRead(sensorP);

  // Substitui valores NaN por 0
  if (isnan(temp)) temp = 0.0;
  if (isnan(pressureInMmHg)) pressureInMmHg = 0.0;
  if (isnan(leitura)) leitura = 0;

  // Verifica se a conexão WiFi está ativa
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    // Criando o JSON de forma segura e garantindo que NaN seja convertido para 0
    String jsonPayload = "{\"temperatura\": " + String(temp, 2) + ", \"ecg\": " + String(leitura) + ", \"pressao\": " + String(pressureInMmHg, 2) + "}";

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
  Serial.println("Leitura ECG: " + String(leitura));
  Serial.println("Temperatura: " + String(temp, 2) + " °C");
  Serial.println("Pressão: " + String(pressureInMmHg, 2) + " mmHg");

  // Aguarda antes de enviar novamente
  delay(1); // Envia a cada 10 segundos
}
