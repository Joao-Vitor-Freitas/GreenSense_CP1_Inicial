# GreenSense — Pinagem do CP1

| Componente | Pino Arduino Mega |
|---|---|
| DHT11 DATA | D7 |
| LDR | A0 |
| LCD RS | D12 |
| LCD EN | D11 |
| LCD D4 | D5 |
| LCD D5 | D4 |
| LCD D6 | D3 |
| LCD D7 | D2 |
| Servo sinal | D9 |
| Botão | D8 |
| LED verde | D13 |
| LED amarelo | D10 |
| LED vermelho | D6 |

### Comportamento do botão
- Toque curto no modo AUTO: troca a tela do LCD.
- Toque longo: alterna AUTO/MANUAL.
- Toque curto no modo MANUAL: alterna o servo entre 0°, 90° e 180°.

### Estados
- NORMAL: LED verde e servo fechado.
- ATENÇÃO: LED amarelo e servo a 90°.
- CRÍTICO: LED vermelho e servo a 180°.
- ERRO: LED vermelho e servo fechado.

> Conferir a pinagem física das peças antes de energizar o circuito.
