/*
 * CardioIA – Fase 3, Parte 1: Edge Computing
 * Aluno : Kleber Foks | RM562225 | 2TIAOA
 *
 * Sensores:
 *   - DHT22  → temperatura e umidade
 *   - Botão  → simula batimentos cardíacos (BPM)
 *
 * Lógica de resiliência offline (Edge Computing):
 *   - Buffer circular de até 500 amostras na RAM (~16 min offline)
 *   - Variável booleana simula conectividade Wi-Fi
 *   - Quando "online": flush do buffer via Serial e limpa buffer
 *   - Quando "offline": continua coletando sem perder dados
 */

#include <DHT.h>

// ─── Pinos ───────────────────────────────────────────────────────────────────
#define DHTPIN    4      // GPIO4 → DHT22
#define DHTTYPE   DHT22
#define BTN_PIN   5      // GPIO5 → Botão (conta cliques = BPM simulado)

// ─── Buffer circular ─────────────────────────────────────────────────────────
#define MAX_SAMPLES 500  // Máximo de amostras offline

struct Amostra {
  unsigned long timestamp;  // millis()
  float temperatura;
  float umidade;
  int   bpm;
  bool  alerta;             // true se temp>38 ou bpm>120
};

Amostra buffer[MAX_SAMPLES];
int bufferHead = 0;   // próximo índice de escrita
int bufferCount = 0;  // quantidade de amostras no buffer

// ─── Estado ──────────────────────────────────────────────────────────────────
DHT dht(DHTPIN, DHTTYPE);

bool wifiConnected   = false;   // simula conectividade
unsigned long lastToggle  = 0;  // controla alternância de Wi-Fi
unsigned long lastRead    = 0;  // controla leitura periódica dos sensores
unsigned long lastBtnTime = 0;  // debounce do botão
int  clickCount      = 0;       // cliques no intervalo de medição de BPM
bool lastBtnState    = HIGH;

// ─── Configurações de tempo ───────────────────────────────────────────────────
const unsigned long READ_INTERVAL   = 2000;   // lê sensores a cada 2 s
const unsigned long WIFI_TOGGLE     = 30000;  // alterna Wi-Fi a cada 30 s
const unsigned long BPM_WINDOW      = 10000;  // janela de contagem de BPM = 10 s

// ─── Protótipos ───────────────────────────────────────────────────────────────
void adicionarAmostra(float t, float u, int bpm);
void flushBuffer();
int  calcularBPM();

// ─────────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(BTN_PIN, INPUT_PULLUP);

  Serial.println("=== CardioIA Edge Computing – Parte 1 ===");
  Serial.println("Formato CSV: timestamp,temperatura,umidade,bpm,alerta");
}

// ─────────────────────────────────────────────────────────────────────────────
void loop() {
  unsigned long agora = millis();

  // ── 1. Debounce e contagem de cliques do botão ──────────────────────────
  bool btnAtual = digitalRead(BTN_PIN);
  if (btnAtual == LOW && lastBtnState == HIGH) {
    clickCount++;
    lastBtnTime = agora;
  }
  lastBtnState = btnAtual;

  // ── 2. Alterna Wi-Fi simulado a cada 30 s ───────────────────────────────
  if (agora - lastToggle >= WIFI_TOGGLE) {
    wifiConnected = !wifiConnected;
    lastToggle    = agora;
    Serial.print("[WiFi] Status: ");
    Serial.println(wifiConnected ? "ONLINE" : "OFFLINE (modo Edge)");
  }

  // ── 3. Leitura de sensores a cada 2 s ───────────────────────────────────
  if (agora - lastRead >= READ_INTERVAL) {
    lastRead = agora;

    float temp = dht.readTemperature();
    float umid = dht.readHumidity();

    // Valida leitura do DHT22
    if (isnan(temp) || isnan(umid)) {
      Serial.println("[ERRO] Falha na leitura do DHT22");
      return;
    }

    // BPM: clicks na janela de 10 s, extrapolado para 1 min
    int bpm = (clickCount * 60) / (BPM_WINDOW / 1000);
    clickCount = 0; // reseta para próxima janela

    // Armazena no buffer (Edge Computing)
    adicionarAmostra(temp, umid, bpm);

    // ── 4. Se online, envia e limpa buffer ──────────────────────────────
    if (wifiConnected) {
      flushBuffer();
    } else {
      Serial.print("[EDGE] Amostra armazenada offline. Buffer: ");
      Serial.print(bufferCount);
      Serial.print("/");
      Serial.println(MAX_SAMPLES);
    }
  }
}

// ─── Adiciona amostra no buffer circular ─────────────────────────────────────
void adicionarAmostra(float t, float u, int bpm) {
  bool alerta = (t > 38.0 || bpm > 120);

  // Se buffer cheio, sobrescreve a amostra mais antiga (circular)
  buffer[bufferHead] = { millis(), t, u, bpm, alerta };
  bufferHead = (bufferHead + 1) % MAX_SAMPLES;
  if (bufferCount < MAX_SAMPLES) bufferCount++;
}

// ─── Envia todas as amostras acumuladas e limpa buffer ────────────────────────
void flushBuffer() {
  if (bufferCount == 0) return;

  Serial.println("[MQTT-SERIAL] Enviando buffer acumulado...");

  // Calcula índice de início (FIFO)
  int inicio = (bufferHead - bufferCount + MAX_SAMPLES) % MAX_SAMPLES;

  for (int i = 0; i < bufferCount; i++) {
    int idx = (inicio + i) % MAX_SAMPLES;
    Amostra& a = buffer[idx];

    // Formato CSV para simulação de envio MQTT via Serial
    Serial.print(a.timestamp);    Serial.print(",");
    Serial.print(a.temperatura);  Serial.print(",");
    Serial.print(a.umidade);      Serial.print(",");
    Serial.print(a.bpm);          Serial.print(",");
    Serial.println(a.alerta ? "ALERTA" : "OK");
  }

  // Limpa buffer após envio
  bufferCount = 0;
  bufferHead  = 0;
  Serial.println("[MQTT-SERIAL] Buffer enviado e limpo.");
}
