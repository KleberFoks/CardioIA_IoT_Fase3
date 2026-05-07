# FIAP - Faculdade de Informática e Administração Paulista

<p align="center">
  <a href="https://www.fiap.com.br/"><img src="assets/logo-fiap.png" alt="FIAP" border="0" width=40% height=40%></a>
</p>

<br>

# CardioIA – Monitoramento Contínuo com IoT

## Fase 3 – IoT na Saúde

## 👨‍🎓 Integrante:
- Kleber Foks — RM562225

## 👩‍🏫 Professores:
### Tutor(a)
- *(informar nome do tutor)*
### Coordenador(a)
- *(informar nome do coordenador)*

---

## 📜 Descrição

O **CardioIA** é um protótipo de sistema vestível de monitoramento cardíaco baseado em **ESP32**, desenvolvido na plataforma Wokwi. O sistema captura sinais vitais simulados (temperatura, umidade e batimentos cardíacos), processa os dados localmente com estratégia de resiliência offline (**Edge Computing**), transmite para a nuvem via **MQTT** e exibe os resultados em um **dashboard Node-RED** com alertas automáticos.

### Sensores utilizados
| Sensor | Dado coletado |
|--------|--------------|
| DHT22 | Temperatura (°C) e Umidade (%) |
| Botão (Push Button) | Batimentos cardíacos simulados (BPM) |

### Arquitetura geral
```
ESP32 (Wokwi) → buffer circular (Edge) → MQTT → Node-RED Dashboard
```

---

## 📁 Estrutura de pastas

```
CardioIA_IoT_Fase3/
├── assets/                         → Imagens e recursos visuais
├── src/
│   ├── parte1_edge/
│   │   ├── cardio_edge.ino         → Código ESP32 – Edge Computing
│   │   └── diagram.json            → Diagrama do circuito (Wokwi)
│   └── parte2_mqtt/
│       ├── cardio_mqtt.ino         → Código ESP32 – MQTT
│       └── node_red_flow.json      → Flow do dashboard Node-RED
├── document/
│   ├── relatorio_parte1_edge.md    → Relatório Parte 1
│   └── relatorio_parte2_mqtt.md    → Relatório Parte 2
└── README.md
```

---

## 🔧 Como executar

### Parte 1 – Wokwi (Edge Computing)
1. Acesse o projeto (ou utilize o código em `src/parte1_edge/`): [https://wokwi.com/projects/463336808659675137](https://wokwi.com/projects/463336808659675137)
2. Clique em ▶️ para simular — observe o Monitor Serial
5. O LED do botão simula cliques de batimento cardíaco

**Bibliotecas necessárias:** `DHT sensor library` by Adafruit

### Parte 2 – MQTT + Node-RED
1. Acesse o projeto: [https://wokwi.com/projects/463341701605795841](https://wokwi.com/projects/463341701605795841)
2. Instale o Node-RED: `npm install -g node-red`
3. Instale o dashboard: `cd ~/.node-red && npm install node-red-dashboard`
4. Inicie: `node-red`
5. Importe `src/parte2_mqtt/node_red_flow.json` via Menu → Import
6. Clique em Deploy e acesse `http://localhost:1880/ui`

**Broker MQTT:** `broker.hivemq.com:1883` (público, sem autenticação)

---

## 🗃 Histórico de lançamentos

* 0.3.0 – 07/05/2026 — Fase 3: IoT, MQTT, Node-RED dashboard
* 0.2.0 – *(Fase 2)*
* 0.1.0 – *(Fase 1)*

---

## 📋 Licença

<img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/cc.svg?ref=chooser-v1"><img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/by.svg?ref=chooser-v1"><p xmlns:cc="http://creativecommons.org/ns#" xmlns:dct="http://purl.org/dc/terms/"><a property="dct:title" rel="cc:attributionURL" href="https://github.com/agodoi/template">MODELO GIT FIAP</a> por <a rel="cc:attributionURL dct:creator" property="cc:attributionName" href="https://fiap.com.br">Fiap</a> está licenciado sobre <a href="http://creativecommons.org/licenses/by/4.0/?ref=chooser-v1" target="_blank" rel="license noopener noreferrer" style="display:inline-block;">Attribution 4.0 International</a>.</p>
