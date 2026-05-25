#include <Ultrasonic.h>
#include <Wire.h>
#include "Adafruit_VL53L1X.h"

// ==========================================
// 1. CONFIGURAÇÕES, PINOS E VARIÁVEIS GLOBAIS
// ==========================================
#define PIN_LINHA_FRONT_ESQ 2
#define PIN_LINHA_FRONT_DIR 3
#define ESTADO_BORDA_PRETA HIGH

#define MOTOR_ESQ1 5
#define MOTOR_ESQ2 6
#define MOTOR_DIR1 10
#define MOTOR_DIR2 9

#define LED_ALERTA 13

// Pinos dos Sensores Ultrassônicos Laterais
#define ULTRA_ESQ_TRIG 14
#define ULTRA_ESQ_ECHO 15
#define ULTRA_DIR_TRIG 16
#define ULTRA_DIR_ECHO 17

// Pinos XSHUT dos Lasers Frontais
#define SENSOR1_XSHUT 4
#define SENSOR2_XSHUT 7
#define SENSOR3_XSHUT 8

enum Estados { INICIAL, PROCURA, ATAQUE, FUGA_LINHA };
Estados estadoAtual = INICIAL;

// VELOCIDADES DEFINIDAS INDEPENDENTEMENTE DA RAMPA
const int VELOCIDADE_BUSCA = 120;
const int VELOCIDADE_ATAQUE = 255;
const int VELOCIDADE_CURVA = 110;
const int VELOCIDADE_GIRO_BRUSCO = 190;

int distMin = 40;  // em milímetros (4 cm)
int distMax = 550; // em milímetros (55 cm)

Adafruit_VL53L1X sensorEsq;
Adafruit_VL53L1X sensorCtr;
Adafruit_VL53L1X sensorDir;

// Variáveis de leitura de distância física
uint16_t distLaserEsq = 0;
uint16_t distLaserCtr = 0;
uint16_t distLaserDir = 0;
long distUltraEsq = 0;
long distUltraDir = 0;

bool lEsq = false;
bool lCtr = false;
bool lDir = false;
bool uEsq = false;
bool uDir = false;

int ultimoLadoVisto = 1;

// Configuração dos Ultrassônicos (Echo, Trig)
Ultrasonic ultraEsq(ULTRA_ESQ_ECHO, ULTRA_ESQ_TRIG);
Ultrasonic ultraDir(ULTRA_DIR_ECHO, ULTRA_DIR_TRIG);

// ==========================================
// CONFIGURAÇÃO INICIAL (SETUP)
// ==========================================
void setup() {
  Serial.begin(115200);
  Wire.begin();

  pinMode(PIN_LINHA_FRONT_DIR, INPUT);
  pinMode(PIN_LINHA_FRONT_ESQ, INPUT);

  configurarMotores();
  inicializarSensoresLaser();
  estadoAtual = INICIAL;
}

void loop() {
  if (verificarBordaDaArena()) {
    estadoAtual = FUGA_LINHA;
  }

  switch (estadoAtual) {
    case INICIAL:
      enviarSinalPonteH(0, 0);
      delay(4500); // Tempo regulamentar de segurança
      estadoAtual = PROCURA;
      break;

    case PROCURA:
      if (deteccaoInicial()) {
        estadoAtual = ATAQUE;
      } else {
        executarProcura();
      }
      break;

    case ATAQUE:
      if (atualizarEDetectouInimigo()) {
        executarAtaque();
      } else {
        estadoAtual = PROCURA;
      }
      break;

    case FUGA_LINHA:
      executarFugaDeLinha();
      estadoAtual = PROCURA;
      break;
  }
}

bool verificarBordaDaArena() {
  return (digitalRead(PIN_LINHA_FRONT_ESQ) == ESTADO_BORDA_PRETA || 
          digitalRead(PIN_LINHA_FRONT_DIR) == ESTADO_BORDA_PRETA);
}

// FASE DE COMBATE: Varredura focada apenas nos sensores Laser frontais
bool atualizarEDetectouInimigo() {
  distLaserEsq = sensorEsq.distance();
  distLaserCtr = sensorCtr.distance();
  distLaserDir = sensorDir.distance();

  lEsq = (distLaserEsq > distMin && distLaserEsq < distMax);
  lCtr = (distLaserCtr > distMin && distLaserCtr < distMax);
  lDir = (distLaserDir > distMin && distLaserDir < distMax);

  if (lEsq) ultimoLadoVisto = -1;
  if (lDir) ultimoLadoVisto = 1;

  return (lEsq || lCtr || lDir);
}

// FASE INICIAL/PROCURA: Varredura periférica expandida com os Ultrassônicos Laterais
bool deteccaoInicial() { 
  distLaserEsq = sensorEsq.distance();
  distLaserCtr = sensorCtr.distance();
  distLaserDir = sensorDir.distance();
  
  // Converte a leitura em cm da biblioteca Ultrasonic para mm (multiplicando por 10)
  distUltraEsq = (ultraEsq.read() * 10);
  distUltraDir = (ultraDir.read() * 10);

  lEsq = (distLaserEsq > distMin && distLaserEsq < distMax);
  lCtr = (distLaserCtr > distMin && distLaserCtr < distMax);
  lDir = (distLaserDir > distMin && distLaserDir < distMax);
  uEsq = (distUltraEsq > distMin && distUltraEsq < distMax);
  uDir = (distUltraDir > distMin && distUltraDir < distMax);

  if (lEsq || uEsq) ultimoLadoVisto = -1;
  if (lDir || uDir) ultimoLadoVisto = 1;

  return (lEsq || lCtr || lDir || uEsq || uDir);
}

void executarProcura() {
  // Gira no próprio eixo usando valores inteiros de velocidade corretos
  if (ultimoLadoVisto == 1) {
    enviarSinalPonteH(VELOCIDADE_BUSCA, -VELOCIDADE_BUSCA); // Gira para a direita
  } else {
    enviarSinalPonteH(-VELOCIDADE_BUSCA, VELOCIDADE_BUSCA); // Gira para a esquerda
  }
}

void executarAtaque() {
  if (lCtr) {
    // Alvo centralizado -> Carga total para frente
    enviarSinalPonteH(VELOCIDADE_ATAQUE, VELOCIDADE_ATAQUE);
  } else if (lEsq) {
    // Alvo na diagonal esquerda -> Curva agressiva para frente/esquerda
    enviarSinalPonteH(VELOCIDADE_CURVA, VELOCIDADE_ATAQUE);
  } else if (lDir) {
    // Alvo na diagonal direita -> Curva agressiva para frente/direita
    enviarSinalPonteH(VELOCIDADE_ATAQUE, VELOCIDADE_CURVA);
  } else if (uEsq) {
    // Alvo detectado na lateral esquerda (Fase Inicial) -> Giro brusco parado para enquadrar
    enviarSinalPonteH(-VELOCIDADE_GIRO_BRUSCO, VELOCIDADE_GIRO_BRUSCO);
  } else if (uDir) {
    // Alvo detectado na lateral direita (Fase Inicial) -> Giro brusco parado para enquadrar
    enviarSinalPonteH(VELOCIDADE_GIRO_BRUSCO, -VELOCIDADE_GIRO_BRUSCO);
  }
}

void executarFugaDeLinha() {
  enviarSinalPonteH(-255, -255);
  delay(280);
  enviarSinalPonteH(255, -255);
  delay(220);
}

void inicializarSensoresLaser() {
  pinMode(SENSOR1_XSHUT, OUTPUT);
  pinMode(SENSOR2_XSHUT, OUTPUT);
  pinMode(SENSOR3_XSHUT, OUTPUT);

  digitalWrite(SENSOR1_XSHUT, LOW);
  digitalWrite(SENSOR2_XSHUT, LOW);
  digitalWrite(SENSOR3_XSHUT, LOW);
  delay(40);

  digitalWrite(SENSOR1_XSHUT, HIGH);
  delay(15);
  if (sensorEsq.begin(0x29, &Wire)) {
    sensorEsq.VL53L1X_SetI2CAddress(0x30);
    sensorEsq.startRanging();
  } else {
    digitalWrite(LED_ALERTA, HIGH);
  }

  digitalWrite(SENSOR2_XSHUT, HIGH);
  delay(15);
  if (sensorCtr.begin(0x29, &Wire)) {
    sensorCtr.VL53L1X_SetI2CAddress(0x27);
    sensorCtr.startRanging();
  } else {
    digitalWrite(LED_ALERTA, HIGH);
  }

  digitalWrite(SENSOR3_XSHUT, HIGH);
  delay(15);
  if (sensorDir.begin(0x29, &Wire)) {
    sensorDir.startRanging();
  } else {
    digitalWrite(LED_ALERTA, HIGH);
  }
}

void configurarMotores() {
  pinMode(MOTOR_ESQ1, OUTPUT);
  pinMode(MOTOR_ESQ2, OUTPUT);
  pinMode(MOTOR_DIR1, OUTPUT);
  pinMode(MOTOR_DIR2, OUTPUT);
}

void enviarSinalPonteH(int esq, int dir) {
  if (esq > 0) {
    analogWrite(MOTOR_ESQ1, esq);
    analogWrite(MOTOR_ESQ2, 0);
  } else if (esq < 0) {
    analogWrite(MOTOR_ESQ1, 0);
    analogWrite(MOTOR_ESQ2, abs(esq));
  } else {
    analogWrite(MOTOR_ESQ1, 0);
    analogWrite(MOTOR_ESQ2, 0);
  }

  if (dir > 0) {
    analogWrite(MOTOR_DIR1, dir);
    analogWrite(MOTOR_DIR2, 0);
  } else if (dir < 0) {
    analogWrite(MOTOR_DIR1, 0);
    analogWrite(MOTOR_DIR2, abs(dir));
  } else {
    analogWrite(MOTOR_DIR1, 0);
    analogWrite(MOTOR_DIR2, 0);
  }
}