# GreenSense — Checkpoint 1

## Objetivo
Protótipo offline de uma estufa inteligente para monitorar temperatura, umidade e luminosidade, mostrar informações localmente e realizar atuação com servo motor.

## Hardware
- Arduino Mega 2560 R3
- DHT11
- LDR
- LCD 16x2
- Microservo 9g
- Push button
- LEDs verde, amarelo e vermelho
- Protoboard
- Resistores
- Jumpers

## Software
- Leitura de DHT11 e LDR
- Classificação NORMAL / ATENÇÃO / CRÍTICO
- Controle do servo
- LCD com múltiplas telas
- Debounce do botão
- Temporização com millis()
- Watchdog do Arduino
- Validação básica do DHT11
- Saída serial estruturada para futura integração com ESP32

## Limites iniciais do protótipo
- Temperatura >= 30 °C: ATENÇÃO
- Temperatura >= 35 °C: CRÍTICO
- Umidade <= 30%: ATENÇÃO
- Umidade <= 20%: CRÍTICO

Os limites são parâmetros de protótipo e deverão ser calibrados durante os testes.

## Próxima fase
Arduino -> ESP32 -> protocolo de comunicação -> Wi-Fi -> MQTT -> FIWARE -> Dashboard.
