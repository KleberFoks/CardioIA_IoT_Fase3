# Relatório – Parte 1: Edge Computing com ESP32

**Projeto:** CardioIA – Monitoramento Cardíaco  
**Aluno:** Kleber Foks | RM562225 | 2TIAOA  
**Data:** 07/05/2026

---

## 1. Visão Geral do Sistema

O sistema CardioIA simula um dispositivo vestível de monitoramento cardíaco baseado em ESP32. Na Parte 1, o foco é o processamento e armazenamento local de dados (*Edge Computing*), garantindo que sinais vitais sejam coletados mesmo sem conexão com a nuvem. O projeto pode ser visualizado no Wokwi: [https://wokwi.com/projects/463336808659675137](https://wokwi.com/projects/463336808659675137).

---

## 2. Sensores Utilizados

| Sensor | Pino | Dados Coletados |
|--------|------|-----------------|
| **DHT22** | GPIO 4 | Temperatura (°C) e Umidade (%) |
| **Botão (Push Button)** | GPIO 5 | Simula batimentos cardíacos (BPM) |

O DHT22 realiza leituras a cada 2 segundos. O botão é pressionado manualmente para simular batimentos: o número de cliques em 10 segundos é extrapolado para BPM (Batimentos Por Minuto).

---

## 3. Fluxo de Funcionamento

```
┌─────────────┐     a cada 2s     ┌─────────────────┐
│  DHT22 +    │ ───────────────>  │  Processa leitura│
│   Botão     │                   │  (Edge ESP32)    │
└─────────────┘                   └────────┬─────────┘
                                           │
                         ┌─────────────────▼────────────────┐
                         │         Wi-Fi conectado?          │
                         └──────┬──────────────┬─────────────┘
                               SIM            NÃO
                                │               │
                    ┌───────────▼──┐    ┌───────▼────────────┐
                    │ Flush buffer │    │  Armazena no buffer │
                    │ via Serial   │    │  circular (RAM)     │
                    │ + Limpa      │    │  até 500 amostras   │
                    └──────────────┘    └────────────────────┘
```

---

## 4. Estratégia de Resiliência Offline (Edge Computing)

### 4.1 Buffer Circular na RAM

Optou-se por um **buffer circular em memória RAM** como alternativa ao SPIFFS (não suportado em simuladores). O buffer armazena structs com os seguintes campos:

```cpp
struct Amostra {
  unsigned long timestamp;  // tempo em ms
  float temperatura;
  float umidade;
  int   bpm;
  bool  alerta;
};
```

### 4.2 Capacidade e Autonomia Offline

| Parâmetro | Valor |
|-----------|-------|
| Intervalo de leitura | 2 segundos |
| Capacidade do buffer | 500 amostras |
| **Autonomia offline** | **~16 minutos** |

**Justificativa dos 500 registros:** Em um ambiente clínico, 16 minutos é tempo suficiente para lidar com quedas de conectividade transitórias (ex.: troca de roteador, zona de sombra Wi-Fi). Para um dispositivo vestível em ambiente hospitalar, esse período cobre a maioria dos cenários de perda de sinal sem comprometer a continuidade do monitoramento. Cada struct ocupa aproximadamente 13 bytes → buffer total ≈ **6,5 KB** de RAM (dentro dos 320 KB disponíveis no ESP32).

### 4.3 Comportamento com Buffer Cheio

Quando o buffer atinge 500 amostras e o dispositivo ainda está offline, a amostra mais antiga é sobrescrita (comportamento FIFO circular). Isso garante que os **dados mais recentes** são sempre preservados, priorizando a relevância clínica das informações.

### 4.4 Sincronização ao Reconectar

Ao detectar Wi-Fi ativo (variável `wifiConnected = true`), o sistema:
1. Itera sobre todas as amostras do buffer em ordem cronológica
2. Envia cada linha via `Serial.println` no formato CSV
3. Limpa o buffer após o envio completo

---

## 5. Formato dos Dados

Saída via Serial Monitor (simulando envio MQTT):

```
timestamp,temperatura,umidade,bpm,alerta
1234567,36.5,60.2,75,OK
1236567,37.8,61.0,130,ALERTA
```

**Threshold de alerta:** temperatura > 38°C **ou** BPM > 120

---

## 6. Conclusão

A abordagem de Edge Computing implementada garante resiliência offline sem depender de armazenamento físico (SD/SPIFFS), viabilizando a solução tanto em simuladores quanto em hardware real. O buffer circular com 500 amostras oferece autonomia adequada para o contexto de monitoramento cardíaco hospitalar, priorizando sempre os dados mais recentes em caso de overflow.
