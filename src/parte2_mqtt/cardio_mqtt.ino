/*
 * CardioIA – Fase 3, Parte 2: MQTT + Cloud
 * Aluno : Kleber Foks | RM562225 | 2TIAOA
 *
 * Broker  : broker.hivemq.com:1883  (público, sem autenticação)
 * Tópicos :
 *   cardio/temperatura  → float
 *   cardio/umidade      → float
 *   cardio/bpm          → int
 *   cardio/alerta       → "OK" | "ALERTA"
 *
 * Sensores:
 *   - DHT22 → temperatura + umidade (GPIO4)
 *   - Botão → conta cliques para simular BPM (GPIO5)
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ─── Wi-Fi ────────────────────────────────────────────────────────────────────
// ATENÇÃO: substitua pelas suas credenciais ao testar em hardware real.
// No Wokwi, Wi-Fi é simulado e não requer credenciais reais.
const char* SSID     = "Wokwi-GUEST";
const char* SENHA    = "";

// ─── MQTT ────────────────────────────────────────────────────────────────────
const char* MQTT_BROKER = "broker.hivemq.com";
const int   MQTT_PORT   = 1883;
const char* CLIENT_ID   = "cardio_kleber_rm562225"; // único por cliente

// Tópicos de publicação
const char* TOP_TEMP   = "cardio/temperatura";
const char* TOP_UMID   = "cardio/umidade";
const char* TOP_BPM    = "cardio/bpm";
const char* TOP_ALERTA = "cardio/alerta";

// ─── Pinos ───────────────────────────────────────────────────────────────────
#define DHTPIN  4
#define DHTTYPE DHT22
#define BTN_PIN 5

// ─── Objetos ─────────────────────────────────────────────────────────────────
WiFiClient   espClient;
PubSubClient mqtt(espClient);
DHT          dht(DHTPIN, DHTTYPE);

// ─── Variáveis de estado ─────────────────────────────────────────────────────
unsigned long lastPublish = 0;
const unsigned long PUB_INTERVAL = 2000; // publica a cada 2 s

int  clickCount   = 0;
bool lastBtnState = HIGH;

// Thresholds de alerta
const float TEMP_MAX = 38.0;
const int   BPM_MAX  = 120;

// ─────────────────────────────────────────────────────────────────────────────
void conectarWiFi() {
  Serial.print("[WiFi] Conectando");
  WiFi.begin(SSID, SENHA);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" OK");
  Serial.print("[WiFi] IP: ");
  Serial.println(WiFi.localIP());
}

void conectarMQTT() {
  while (!mqtt.connected()) {
    Serial.print("[MQTT] Conectando ao broker...");
    if (mqtt.connect(CLIENT_ID)) {
      Serial.println(" conectado!");
    } else {
      Serial.print(" falhou (rc=");
      Serial.print(mqtt.state());
      Serial.println("). Tentando em 3 s...");
      delay(3000);
    }
  }
}

// ─────────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(BTN_PIN, INPUT_PULLUP);

  conectarWiFi();
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);

  Serial.println("=== CardioIA MQTT – Parte 2 ===");
}

// ─────────────────────────────────────────────────────────────────────────────
void loop() {
  // Mantém conexão MQTT ativa
  if (!mqtt.connected()) conectarMQTT();
  mqtt.loop();

  // Contagem de cliques do botão (BPM)
  bool btnAtual = digitalRead(BTN_PIN);
  if (btnAtual == LOW && lastBtnState == HIGH) clickCount++;
  lastBtnState = btnAtual;

  // Publica a cada 2 s
  if (millis() - lastPublish >= PUB_INTERVAL) {
    lastPublish = millis();

    float temp = dht.readTemperature();
    float umid = dht.readHumidity();

    if (isnan(temp) || isnan(umid)) {
      Serial.println("[ERRO] Falha DHT22");
      return;
    }

    // BPM: cliques em 2 s → extrapola para 1 min
    int bpm = clickCount * 30;
    clickCount = 0;

    bool alerta = (temp > TEMP_MAX || bpm > BPM_MAX);

    // Publica cada tópico como string
    char buf[16];

    dtostrf(temp, 4, 1, buf);
    mqtt.publish(TOP_TEMP, buf);

    dtostrf(umid, 4, 1, buf);
    mqtt.publish(TOP_UMID, buf);

    snprintf(buf, sizeof(buf), "%d", bpm);
    mqtt.publish(TOP_BPM, buf);

    mqtt.publish(TOP_ALERTA, alerta ? "ALERTA" : "OK");

    // Log no Serial
    Serial.print("[PUB] Temp="); Serial.print(temp);
    Serial.print(" Umid=");      Serial.print(umid);
    Serial.print(" BPM=");       Serial.print(bpm);
    Serial.print(" Alerta=");    Serial.println(alerta ? "SIM" : "NAO");
  }
}
