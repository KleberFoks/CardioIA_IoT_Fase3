# Relatório – Parte 2: MQTT e Dashboard Node-RED

**Projeto:** CardioIA – Monitoramento Cardíaco  
**Aluno:** Kleber Foks | RM562225 | 2TIAOA  
**Data:** 07/05/2026

---

## 1. Visão Geral

A Parte 2 integra o ESP32 com a nuvem por meio do protocolo **MQTT**, publicando sinais vitais em tempo real para um broker público. O projeto pode ser visualizado no Wokwi: [https://wokwi.com/projects/463341701605795841](https://wokwi.com/projects/463341701605795841).

---

## 2. Arquitetura MQTT

```
┌─────────────┐   publish    ┌──────────────────────┐   subscribe   ┌───────────────┐
│   ESP32     │ ──────────>  │  broker.hivemq.com   │ ────────────> │   Node-RED    │
│ (Publisher) │   porta 1883 │  (Broker Público)    │               │ (Subscriber)  │
└─────────────┘              └──────────────────────┘               └───────┬───────┘
                                                                             │
                                                                    ┌────────▼────────┐
                                                                    │ Dashboard HTTP  │
                                                                    │ localhost:1880  │
                                                                    └─────────────────┘
```

### 2.1 Por que MQTT?

MQTT (*Message Queuing Telemetry Transport*) é o protocolo padrão para IoT por três razões principais:

1. **Leveza:** cabeçalho mínimo de 2 bytes, ideal para microcontroladores
2. **Pub/Sub:** desacopla produtor e consumidor; o ESP32 não precisa conhecer quem consome os dados
3. **QoS configurável:** garante entrega mesmo em redes instáveis (QoS 1 = entrega ao menos uma vez)

---

## 3. Tópicos MQTT Publicados

| Tópico | Tipo | Exemplo | Frequência |
|--------|------|---------|------------|
| `cardio/temperatura` | float | `36.5` | 2 s |
| `cardio/umidade` | float | `60.2` | 2 s |
| `cardio/bpm` | int | `78` | 2 s |
| `cardio/alerta` | string | `OK` / `ALERTA` | 2 s |

### 3.1 Thresholds de Alerta

| Parâmetro | Limite | Ação |
|-----------|--------|------|
| Temperatura | > 38°C | Publica `ALERTA` em `cardio/alerta` |
| BPM | > 120 bpm | Publica `ALERTA` em `cardio/alerta` |

---

## 4. Configuração do Broker

**Broker:** `broker.hivemq.com:1883`  
**Autenticação:** nenhuma (broker público, para fins de desenvolvimento)  
**Client IDs:** `cardio_kleber_rm562225` (ESP32) e `nodered_cardio_rm562225` (Node-RED)

> **Boas práticas em produção:** Em ambiente real hospitalar, deve-se usar TLS (porta 8883) com autenticação por certificado X.509, broker privado e ACLs (Access Control Lists) por tópico.

---

## 5. Dashboard Node-RED

### 5.1 Importação do Flow

1. Abrir Node-RED em `http://localhost:1880`
2. Menu → *Import* → colar conteúdo de `node_red_flow.json`
3. Instalar o pacote `node-red-dashboard` se não instalado:
   ```
   cd ~/.node-red
   npm install node-red-dashboard
   ```
4. Fazer *Deploy* e acessar `http://localhost:1880/ui`

### 5.2 Widgets do Dashboard

| Widget | Tipo | Tópico | Descrição |
|--------|------|--------|-----------|
| **Temperatura** | Gauge (medidor) | `cardio/temperatura` | Escala 35–42°C; vermelho acima de 38°C |
| **BPM** | Chart (linha) | `cardio/bpm` | Histórico dos últimos 60 pontos |
| **Status** | Text | `cardio/alerta` | ✅ Sinais Normais / 🚨 ALERTA CARDÍACO! |

### 5.3 Lógica de Alerta no Node-RED

O nó *function* "Formata Alerta" recebe o payload do tópico `cardio/alerta` e formata a mensagem:

```javascript
if (msg.payload === 'ALERTA') {
  msg.payload = '🚨 ALERTA CARDÍACO!';
} else {
  msg.payload = '✅ Sinais Normais';
}
return msg;
```

---

## 6. Fluxo Completo de Dados

```
DHT22 lê temp/umid → ESP32 calcula BPM → verifica thresholds
    → publica 4 tópicos MQTT → broker.hivemq.com
    → Node-RED subscreve → Gauge | Chart | Texto de alerta
```

Frequência de atualização: **2 segundos** (tempo real para monitoramento cardíaco).

---

## 7. Segurança e Boas Práticas em IoT Médico

| Aspecto | Implementação Atual (Dev) | Recomendação em Produção |
|---------|--------------------------|--------------------------|
| Autenticação | Nenhuma (broker público) | Certificado X.509 + TLS 1.3 |
| Criptografia | Não (porta 1883) | TLS (porta 8883) |
| Acesso aos dados | Público | ACL por tópico por paciente |
| Identificação do dispositivo | Client ID fixo | UUID único por dispositivo |
| Conformidade | — | LGPD / HIPAA para dados de saúde |

---

## 8. Conclusão

A integração ESP32 → MQTT → Node-RED demonstra o fluxo completo de uma solução de IoT médico, desde a captura de sinais vitais até a visualização em tempo real com alertas automáticos. A arquitetura pub/sub do MQTT é especialmente adequada para saúde digital por ser leve, escalável e resiliente a falhas de rede. A próxima etapa natural seria a integração com **Grafana Cloud** para histórico de longo prazo e análises preditivas.
