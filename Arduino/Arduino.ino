#include <DHT.h>
#include <LiquidCrystal.h>
#include <Servo.h>
#include <avr/wdt.h>

#define DHT_PIN 7
#define DHT_TYPE DHT11
#define LDR_PIN A0

#define LCD_RS 12
#define LCD_EN 11
#define LCD_D4 5
#define LCD_D5 4
#define LCD_D6 3
#define LCD_D7 2

#define SERVO_PIN 9
#define BUTTON_PIN 8

#define LED_GREEN 13
#define LED_YELLOW 10
#define LED_RED 6

const float TEMP_ATTENTION = 30.0;
const float TEMP_CRITICAL = 35.0;
const float HUM_ATTENTION = 30.0;
const float HUM_CRITICAL = 20.0;

const unsigned long SENSOR_INTERVAL_MS = 2000;
const unsigned long LCD_INTERVAL_MS = 500;
const unsigned long BUTTON_DEBOUNCE_MS = 50;
const unsigned long LONG_PRESS_MS = 1200;

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
Servo ventilacao;

struct DadosEstufa {
  float temperatura = NAN;
  float umidade = NAN;
  int luminosidade = 0;
  bool sensorOk = false;
  String status = "ERRO";
  bool modoAutomatico = true;
  int anguloServo = 0;
};

DadosEstufa dados;

unsigned long ultimoSensor = 0;
unsigned long ultimoLCD = 0;
bool lastButtonReading = HIGH;
bool stableButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long buttonPressStart = 0;
bool longPressHandled = false;
uint8_t telaAtual = 0;

void configurarPinos() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
}

void inicializarDisplay() {
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("GreenSense");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");
  delay(1000); // Apenas tela inicial; temporizacao do sistema usa millis().
}

void calcularStatus() {
  if (!dados.sensorOk) {
    dados.status = "ERRO";
    return;
  }

  if (dados.temperatura >= TEMP_CRITICAL || dados.umidade <= HUM_CRITICAL)
    dados.status = "CRITICO";
  else if (dados.temperatura >= TEMP_ATTENTION || dados.umidade <= HUM_ATTENTION)
    dados.status = "ATENCAO";
  else
    dados.status = "NORMAL";
}

void lerSensores() {
  float t = dht.readTemperature();
  float u = dht.readHumidity();

  dados.luminosidade = analogRead(LDR_PIN);

  if (isnan(t) || isnan(u)) {
    dados.sensorOk = false;
    dados.status = "ERRO";
    return;
  }

  dados.sensorOk = true;
  dados.temperatura = t;
  dados.umidade = u;
  calcularStatus();
}

void atualizarAtuador() {
  if (!dados.sensorOk || dados.modoAutomatico) {
    if (!dados.modoAutomatico)
      return;

    if (!dados.sensorOk) dados.anguloServo = 0;
    else if (dados.status == "CRITICO") dados.anguloServo = 180;
    else if (dados.status == "ATENCAO") dados.anguloServo = 90;
    else dados.anguloServo = 0;

    ventilacao.write(dados.anguloServo);
    return;
  }

  ventilacao.write(dados.anguloServo);
}

void atualizarLEDs() {
  digitalWrite(LED_GREEN, dados.status == "NORMAL");
  digitalWrite(LED_YELLOW, dados.status == "ATENCAO");
  digitalWrite(LED_RED, dados.status == "CRITICO" || dados.status == "ERRO");
}

void atualizarDisplay() {
  lcd.clear();

  if (telaAtual == 0) {
    if (dados.sensorOk) {
      lcd.setCursor(0, 0);
      lcd.print("T:");
      lcd.print(dados.temperatura, 1);
      lcd.print((char)223);
      lcd.print("C U:");
      lcd.print(dados.umidade, 0);
      lcd.print("%");
    } else {
      lcd.setCursor(0, 0);
      lcd.print("ERRO DHT11");
    }

    lcd.setCursor(0, 1);
    lcd.print("Luz:");
    lcd.print(dados.luminosidade);
  } else if (telaAtual == 1) {
    lcd.setCursor(0, 0);
    lcd.print("Status:");
    lcd.print(dados.status);
    lcd.setCursor(0, 1);
    lcd.print("Modo:");
    lcd.print(dados.modoAutomatico ? "AUTO" : "MANUAL");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Servo:");
    lcd.print(dados.anguloServo);
    lcd.print((char)223);
    lcd.setCursor(0, 1);
    lcd.print("Sensores:");
    lcd.print(dados.sensorOk ? "OK" : "ERRO");
  }
}

void trocarTela() {
  telaAtual = (telaAtual + 1) % 3;
}

void alternarModo() {
  dados.modoAutomatico = !dados.modoAutomatico;
  atualizarAtuador();
}

void comandoManualServo() {
  if (dados.modoAutomatico) {
    trocarTela();
    return;
  }

  if (dados.anguloServo == 0) dados.anguloServo = 90;
  else if (dados.anguloServo == 90) dados.anguloServo = 180;
  else dados.anguloServo = 0;

  ventilacao.write(dados.anguloServo);
}

void lerBotao() {
  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonReading)
    lastDebounceTime = millis();

  if (millis() - lastDebounceTime > BUTTON_DEBOUNCE_MS) {
    if (reading != stableButtonState) {
      stableButtonState = reading;

      if (stableButtonState == LOW) {
        buttonPressStart = millis();
        longPressHandled = false;
      } else {
        unsigned long duracao = millis() - buttonPressStart;
        if (duracao < LONG_PRESS_MS && !longPressHandled)
          comandoManualServo();
      }
    }

    if (stableButtonState == LOW &&
        !longPressHandled &&
        millis() - buttonPressStart >= LONG_PRESS_MS) {
      longPressHandled = true;
      alternarModo();
    }
  }

  lastButtonReading = reading;
}

void atualizarSaidaSerial() {
  Serial.print("TEMP=");
  Serial.print(dados.temperatura, 1);
  Serial.print(";UMID=");
  Serial.print(dados.umidade, 0);
  Serial.print(";LDR=");
  Serial.print(dados.luminosidade);
  Serial.print(";STATUS=");
  Serial.print(dados.status);
  Serial.print(";MODO=");
  Serial.print(dados.modoAutomatico ? "AUTO" : "MANUAL");
  Serial.print(";SERVO=");
  Serial.println(dados.anguloServo);
}

void setup() {
  Serial.begin(9600);
  configurarPinos();
  dht.begin();
  ventilacao.attach(SERVO_PIN);
  ventilacao.write(0);
  inicializarDisplay();

  // Watchdog obrigatório no Arduino.
  wdt_enable(WDTO_2S);
}

void loop() {
  unsigned long agora = millis();
  wdt_reset();

  if (ultimoSensor == 0 || agora - ultimoSensor >= SENSOR_INTERVAL_MS) {
    ultimoSensor = agora;
    lerSensores();
    atualizarAtuador();
    atualizarLEDs();
    atualizarSaidaSerial();
  }

  lerBotao();

  if (ultimoLCD == 0 || agora - ultimoLCD >= LCD_INTERVAL_MS) {
    ultimoLCD = agora;
    atualizarDisplay();
  }
}
