# Monitor de Temperatura Corporal com IoT 

Este projeto implementa um **monitor de temperatura corporal sem contato** usando **NodeMCU ESP8266** e **sensor MLX90614**, transmitindo dados em tempo real via **protocolo MQTT** para dashboards online. Um **LED atua como alerta visual** quando a temperatura ultrapassa o limite de febre (37.5 °C).

---

## **Descrição do Projeto**

O sistema realiza leituras de temperatura corporal sem contato utilizando radiação infravermelha. As leituras são enviadas a um broker MQTT em tempo real, podendo ser visualizadas em um dashboard web. O LED indica alertas de temperatura elevada localmente.  

Objetivo: criar uma solução **acessível, de baixo custo e replicável** para monitoramento remoto de temperatura, útil para ambientes domésticos e escolares.

---

## **Materiais e Hardware**

- **NodeMCU ESP8266** – microcontrolador com Wi-Fi integrado.
- **Sensor MLX90614** – termômetro infravermelho digital (I2C).
- **LED 5mm com resistor 220 Ω** – atuador visual de alerta.
- **Dois resistores 4.7 kΩ** – pull-up para barramento I2C (SDA e SCL).
- **Protoboard e jumpers** – montagem sem solda.

---

## **Montagem do Hardware**

1. **MLX90614 → NodeMCU**
   - SDA → D2 (com resistor 4.7 kΩ pull-up conectado ao 3.3 V)
   - SCL → D1 (com resistor 4.7 kΩ pull-up conectado ao 3.3 V)
   - VCC → 3.3 V
   - GND → GND

2. **LED com resistor → NodeMCU**
   - Anôdo do LED → pino D3 
   - Resistor 220 Ω em série
   - Cátodo → GND

3. **Protoboard e jumpers**
   - Organizar conexões sem solda.

---

## **Protocolo de Comunicação**

- **MQTT (Message Queuing Telemetry Transport)**
  - Broker: HiveMQ Cloud (ou outro compatível)
  - Tópico: `ravena/temperatura`
  - Comunicação segura via **WebSockets TLS**
- NodeMCU envia as leituras do MLX90614 ao broker MQTT.
- Dashboard web recebe os dados e atualiza gráfico em tempo real.
- LED acende automaticamente se temperatura > 37.5 °C.

---

## **Software**

### Dependências:

- Arduino IDE
- Bibliotecas:
  - `Wire`
  - `Adafruit_MLX90614`
  - `PubSubClient`

### Código-fonte (NodeMCU):

```cpp
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_MLX90614.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
float ultimaTempValida = 0;
#define LED D3

const char* ssid = "SeuSSID";
const char* password = "SuaSenha";
const char* mqttServer = "seu-broker-hivemq.cloud";
const int mqttPort = 8883;
const char* mqttUser = "usuario";
const char* mqttPassword = "senha";

WiFiClientSecure espClient;
PubSubClient client(espClient);

void setupWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(400);
}

void reconnectMQTT() {
  while (!client.connected()) {
    client.connect("NodeMCUClient", mqttUser, mqttPassword);
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(D2, D1);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  mlx.begin();
  setupWiFi();
  espClient.setInsecure();
  client.setServer(mqttServer, mqttPort);
}

void loop() {
  if (!client.connected()) reconnectMQTT();
  client.loop();

  float leitura = mlx.readObjectTempC();
  if (leitura > 0 && leitura < 50) ultimaTempValida = leitura;

  digitalWrite(LED, ultimaTempValida > 37.5 ? HIGH : LOW);

  char payload[8];
  dtostrf(ultimaTempValida, 4, 2, payload);
  client.publish("ravena/temperatura", payload);

  delay(800);
}
```
---
## **Dashboard Web**

- Código HTML/JS utilizando Chart.js para gráfico de temperatura.

- Cores do gráfico indicam normalidade (verde) ou febre (vermelho).

- Exibe temperatura atual em tempo real e status de conexão MQTT.

---

## **Como Reproduzir**

1. Montar hardware conforme diagrama acima.

2. Configurar NodeMCU com Arduino IDE.

3. Ajustar credenciais Wi-Fi e MQTT no código.

4. Conectar ao broker MQTT (HiveMQ Cloud, Mosquitto ou EMQX).

5. Abrir dashboard web (index.html) em navegador.

6. Aproximar sensor do corpo e observar leituras e LED.

---

## **Repositório Contém**

- Código-fonte NodeMCU completo.

- Dashboard web com gráficos coloridos.

- Documentação de hardware e montagem.

- Explicação da integração MQTT.

- Instruções para reprodução do projeto.

---
## **Contato**

Em caso de dúvidas, entre em contato com a equipe de desenvolvimento:

Dávila Ravena Silva Dorta.

Johnny Kevin Teodoro Costa.

E-mail: 10424743@mackenzista.com.br

---

## **Referências**

MELEXIS. MLX90614 Infrared Thermometer Sensor – Datasheet, 2020.

NODEMCU. NodeMCU Official Website, 2025.

IBM. MQTT: Why it’s good for IoT, 2020.

CREE INC. LED Datasheet, 2019.

RANDOM NERD TUTORIALS. ESP8266 MLX90614 Tutorials, 2024.

SPARKFUN. Protoboards e Resistores, 2019-2021.

goBILDA. Female to Female Jumper Wire, 2025.
