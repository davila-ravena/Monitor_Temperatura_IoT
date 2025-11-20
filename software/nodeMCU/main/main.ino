#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_MLX90614.h>

// ===== CONFIG SENSOR =====
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
float ultimaTempValida = 0;

// ===== PINO DO LED =====
#define LED D3

// ===== CONFIG WIFI =====
const char* ssid = "Ravena 2G";
const char* password = "41956032851";

// ===== CONFIG MQTT =====
const char* mqttServer = "575a6b7357464859a796ab1c3cab0064.s1.eu.hivemq.cloud";
const int mqttPort = 8883;
const char* mqttUser = "ravena";
const char* mqttPassword = "NodeMCU2025#";
const char* topic = "ravena/temperatura";

WiFiClientSecure espClient;
PubSubClient client(espClient);

// ===== INTERVALO DE LEITURA =====
unsigned long lastTime = 0;
const long interval = 800; // ms

// ===== FUNÇÃO CONECTAR WIFI =====
void setupWiFi() {
  Serial.print("Conectando ao WiFi ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
}

// ===== FUNÇÃO CONECTAR MQTT =====
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT... ");
    if (client.connect("NodeMCUClient", mqttUser, mqttPassword)) {
      Serial.println("Conectado!");
    } else {
      Serial.print("Falhou, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 3s...");
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  Wire.begin(D2, D1); // I2C -> SDA = D2, SCL = D1

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  if (!mlx.begin()) {
    Serial.println("Erro ao iniciar MLX90614!");
    while (1);
  }

  setupWiFi();
  espClient.setInsecure(); // TLS sem certificado
  client.setServer(mqttServer, mqttPort);

  Serial.println("Sistema inicializado!");
}

void loop() {
  // ===== RECONEXÃO WIFI =====
  if (WiFi.status() != WL_CONNECTED) setupWiFi();

  // ===== RECONEXÃO MQTT =====
  if (!client.connected()) reconnectMQTT();
  client.loop();

  unsigned long now = millis();
  if (now - lastTime >= interval) {
    lastTime = now;

    // ===== MEDIÇÃO TEMPO DO SENSOR =====
    unsigned long tempoInicioSensor = micros();
    float leitura;
    int tentativas = 0;

    do {
      leitura = mlx.readObjectTempC();
      tentativas++;
      if (isnan(leitura) || leitura < 20 || leitura > 45) delay(50);
    } while ((isnan(leitura) || leitura < 20 || leitura > 45) && tentativas < 3);

    unsigned long tempoFimSensor = micros();
    unsigned long tempoSensor = tempoFimSensor - tempoInicioSensor;

    Serial.print("Tempo de resposta MLX90614: ");
    Serial.print(tempoSensor);
    Serial.println(" us");

    if (isnan(leitura) || leitura < 20 || leitura > 45) {
      Serial.print("Leitura inválida após 3 tentativas: ");
      Serial.println(leitura);
      return; 
    }

    ultimaTempValida = leitura;

    Serial.print("Temperatura Corporal: ");
    Serial.print(ultimaTempValida);
    Serial.println(" °C");

    // ===== MEDIÇÃO TEMPO DO LED =====
    unsigned long tempoInicioLED = micros();
    
    if (ultimaTempValida > 37.5) {
      digitalWrite(LED, HIGH);
      Serial.println("⚠️ Febre detectada!");
    } else {
      digitalWrite(LED, LOW);
    }

    unsigned long tempoFimLED = micros();
    unsigned long tempoLED = tempoFimLED - tempoInicioLED;

    Serial.print("Tempo de resposta LED: ");
    Serial.print(tempoLED);
    Serial.println(" us");

    // ===== PUBLICAÇÃO MQTT =====
    char payload[8];
    dtostrf(ultimaTempValida, 4, 2, payload);
    client.publish(topic, payload);
  }
}
