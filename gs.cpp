// Inclusão das bibliotecas necessárias
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// --- Configurações de Rede e MQTT ---
const char* SSID = "Wokwi-GUEST"; // Nome da rede Wi-Fi (padrão do Wokwi)
const char* PASSWORD = ""; // Senha da rede (deixe em branco para o Wokwi)
const char* MQTT_BROKER = "broker.hivemq.com"; // Broker MQTT público para teste
const int MQTT_PORT = 1883; // Porta padrão do MQTT

// Tópicos MQTT para publicação dos dados
const char* MQTT_TOPIC_TEMP = "fiap/gs/bem_estar/temperatura";
const char* MQTT_TOPIC_UMIDADE = "fiap/gs/bem_estar/umidade";
const char* MQTT_TOPIC_LUZ = "fiap/gs/bem_estar/luminosidade";
const char* MQTT_TOPIC_DISTANCIA = "fiap/gs/bem_estar/distancia";
const char* MQTT_TOPIC_STATUS = "fiap/gs/bem_estar/status";

// --- Configuração dos Pinos do ESP32 ---

// Sensor de Temperatura e Umidade (DHT22)
#define DHT_PIN 15
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// Sensor de Luminosidade (LDR)
#define LDR_PIN 34

// Sensor Ultrassônico (HC-SR04)
#define TRIG_PIN 12
#define ECHO_PIN 14

// Atuadores (LEDs e Buzzer)
#define LED_VERDE_PIN 25  // LED de Trabalho
#define LED_AMARELO_PIN 26 // LED de Alerta
#define LED_VERMELHO_PIN 27 // LED de Pausa
#define BUZZER_PIN 13

// --- Variáveis de Controle do Sistema ---

// Variáveis para a Técnica Pomodoro (em milissegundos)
// Para teste, os valores estão reduzidos. Mude para os valores reais depois.
unsigned long tempoTrabalho = 25 * 1000; // 25 segundos para teste
unsigned long tempoPausa = 10 * 1000;    // 10 segundos para teste
unsigned long tempoAlertaPausa = tempoTrabalho - (5 * 1000); // Alerta 5 seg antes

// Variáveis de estado
enum Estado { TRABALHANDO, PAUSANDO, ALERTA_PAUSA };
Estado estadoAtual = TRABALHANDO;
unsigned long tempoInicioEstado = 0;

// Variáveis para controle de tempo (timers não bloqueantes)
unsigned long ultimoTempoLeituraSensores = 0;
unsigned long ultimoTempoVerificacaoPostura = 0;
unsigned long tempoPosturaInadequada = 0;
unsigned long tempoInicioPosturaInadequada = 0;

// Clientes Wi-Fi e MQTT
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// --- Funções Auxiliares ---

void conectaWiFi() {
  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void reconectaMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Conectando ao Broker MQTT...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("Conectado!");
      mqttClient.publish(MQTT_TOPIC_STATUS, "Sistema Online");
    } else {
      Serial.print("Falha, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

float lerDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duracao = pulseIn(ECHO_PIN, HIGH);
  return duracao * 0.034 / 2;
}

void publicarDadosMQTT(const char* topic, float value) {
  char msg[8];
  dtostrf(value, 1, 2, msg);
  mqttClient.publish(topic, msg);
}

void controlarLEDs() {
  switch (estadoAtual) {
    case TRABALHANDO:
      digitalWrite(LED_VERDE_PIN, HIGH);
      digitalWrite(LED_AMARELO_PIN, LOW);
      digitalWrite(LED_VERMELHO_PIN, LOW);
      break;
    case ALERTA_PAUSA:
      digitalWrite(LED_VERDE_PIN, LOW);
      digitalWrite(LED_AMARELO_PIN, HIGH);
      digitalWrite(LED_VERMELHO_PIN, LOW);
      break;
    case PAUSANDO:
      digitalWrite(LED_VERDE_PIN, LOW);
      digitalWrite(LED_AMARELO_PIN, LOW);
      digitalWrite(LED_VERMELHO_PIN, HIGH);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LDR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_VERDE_PIN, OUTPUT);
  pinMode(LED_AMARELO_PIN, OUTPUT);
  pinMode(LED_VERMELHO_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  dht.begin();
  
  conectaWiFi();
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  
  tempoInicioEstado = millis();
  estadoAtual = TRABALHANDO;
  Serial.println("Iniciando ciclo de trabalho.");
}

void loop() {
  if (!mqttClient.connected()) {
    reconectaMQTT();
  }
  mqttClient.loop();

  unsigned long tempoAtual = millis();

  switch (estadoAtual) {
    case TRABALHANDO:
      if (tempoAtual - tempoInicioEstado >= tempoAlertaPausa && tempoAtual - tempoInicioEstado < tempoTrabalho) {
        estadoAtual = ALERTA_PAUSA;
        Serial.println("Alerta: Pausa em breve.");
        mqttClient.publish(MQTT_TOPIC_STATUS, "Alerta: Pausa em breve");
      }
      break;
      
    case ALERTA_PAUSA:
       if (tempoAtual - tempoInicioEstado >= tempoTrabalho) {
        estadoAtual = PAUSANDO;
        tempoInicioEstado = tempoAtual;
        Serial.println("Hora da pausa!");
        mqttClient.publish(MQTT_TOPIC_STATUS, "Pausa iniciada");
        digitalWrite(BUZZER_PIN, HIGH); delay(500); digitalWrite(BUZZER_PIN, LOW);
      }
      break;

    case PAUSANDO:
      if (tempoAtual - tempoInicioEstado >= tempoPausa) {
        estadoAtual = TRABALHANDO;
        tempoInicioEstado = tempoAtual;
        Serial.println("Fim da pausa. Voltando ao trabalho.");
        mqttClient.publish(MQTT_TOPIC_STATUS, "Ciclo de trabalho iniciado");
        digitalWrite(BUZZER_PIN, HIGH); delay(200); digitalWrite(BUZZER_PIN, LOW);
      }
      break;
  }
  controlarLEDs();

  if (tempoAtual - ultimoTempoLeituraSensores > 5000) { // Leitura a cada 5s
    ultimoTempoLeituraSensores = tempoAtual;
    
    float temp = dht.readTemperature();
    float umidade = dht.readHumidity();
    float luminosidade = analogRead(LDR_PIN);
    float distancia = lerDistancia();

    if (!isnan(temp) && !isnan(umidade)) {
      Serial.printf("Temp: %.1f C, Umidade: %.1f %%, Luz: %.0f, Dist: %.1f cm\n", temp, umidade, luminosidade, distancia);
      
      publicarDadosMQTT(MQTT_TOPIC_TEMP, temp);
      publicarDadosMQTT(MQTT_TOPIC_UMIDADE, umidade);
      publicarDadosMQTT(MQTT_TOPIC_LUZ, luminosidade);
      publicarDadosMQTT(MQTT_TOPIC_DISTANCIA, distancia);

      if (temp < 20.0 || temp > 25.0) mqttClient.publish(MQTT_TOPIC_STATUS, "Alerta: Temperatura fora do conforto!");
      if (luminosidade < 1000) mqttClient.publish(MQTT_TOPIC_STATUS, "Alerta: Ambiente muito escuro!");
    }
  }

  if (tempoAtual - ultimoTempoVerificacaoPostura > 2000) {
    ultimoTempoVerificacaoPostura = tempoAtual;
    float dist = lerDistancia();

    if (dist < 30.0 || dist > 50.0) {
      if (tempoInicioPosturaInadequada == 0) {
        tempoInicioPosturaInadequada = tempoAtual;
      }
      tempoPosturaInadequada = tempoAtual - tempoInicioPosturaInadequada;

      if (tempoPosturaInadequada > 10000) { // Alerta após 10s de postura ruim
        Serial.println("Alerta de Postura!");
        mqttClient.publish(MQTT_TOPIC_STATUS, "Alerta: Postura inadequada!");
        digitalWrite(BUZZER_PIN, HIGH); delay(100); digitalWrite(BUZZER_PIN, LOW);
        tempoInicioPosturaInadequada = 0;
      }
    } else {
      tempoInicioPosturaInadequada = 0;
    }
  }
}