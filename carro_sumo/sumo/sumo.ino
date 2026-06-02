#include <Wire.h>
#include "Adafruit_VL53L1X.h"

  #define PIN_LINHA_FRONT_ESQ 12
  #define PIN_LINHA_FRONT_DIR 2
  #define ESTADO_BORDA_PRETA HIGH

  #define MOTOR_ESQ1 5
  #define MOTOR_ESQ2 6
  #define MOTOR_DIR1 9
  #define MOTOR_DIR2 10

  #define LED_ALERTA 13

  #define SENSOR1_XSHUT 7
  #define SENSOR2_XSHUT 11
  #define SENSOR3_XSHUT 8

enum Estados { INICIAL, PROCURA, ATAQUE, FUGA_LINHA };
Estados estadoAtual = INICIAL;

const int VELOCIDADE_BUSCA = 255;
const int VELOCIDADE_ATAQUE = 255;
const int VELOCIDADE_CURVA = -255;

int distMin = 0; 
int distMax = 550;

Adafruit_VL53L1X sensorEsq;
Adafruit_VL53L1X sensorCtr;
Adafruit_VL53L1X sensorDir;

uint16_t distLaserEsq = 0;
uint16_t distLaserCtr = 0;
uint16_t distLaserDir = 0;

bool lEsq = false;
bool lCtr = false;
bool lDir = false;

int ultimoLadoVisto = 1;

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
      // delay(1600);
      enviarSinalPonteH(0, 0);
      estadoAtual = PROCURA;
      Serial.println("inicial");
      break;

    case PROCURA:
      Serial.println("procura");

      if (atualizarEDetectouInimigo()) {
        estadoAtual = ATAQUE;
      } else {
        executarProcura();
      }
      break;

    case ATAQUE:
      Serial.println("ataque");
      if (atualizarEDetectouInimigo()) {
        executarAtaque();
      } else {
        estadoAtual = PROCURA;
      }
      break;

    case FUGA_LINHA:
      Serial.println("fuga");
      if(digitalRead(PIN_LINHA_FRONT_ESQ) == HIGH){
        executarFugaDeLinhaD();
      }else if(digitalRead(PIN_LINHA_FRONT_DIR) == HIGH){
        executarFugaDeLinhaE();
      }
      estadoAtual = PROCURA;
      break;
  }
}

bool verificarBordaDaArena() {
  return (digitalRead(PIN_LINHA_FRONT_ESQ) == ESTADO_BORDA_PRETA || 
          digitalRead(PIN_LINHA_FRONT_DIR) == ESTADO_BORDA_PRETA);
}

bool atualizarEDetectouInimigo() {
  distLaserEsq = sensorEsq.distance();
  distLaserCtr = sensorCtr.distance();
  distLaserDir = sensorDir.distance();

  lEsq = (distLaserEsq > distMin && distLaserEsq < distMax);
  lCtr = (distLaserCtr > distMin && distLaserCtr < distMax);
  lDir = (distLaserDir > distMin && distLaserDir < distMax);

  Serial.print("E :");
  Serial.print(lEsq);
  Serial.print(" | ");
  Serial.print("C :");
  Serial.print(lCtr);
  Serial.print(" | ");
  Serial.print("D :");
  Serial.println(lDir);

  if (lEsq) ultimoLadoVisto = -1;
  if (lDir) ultimoLadoVisto = 1;

  return (lEsq || lCtr || lDir);
}

void executarProcura() {
  if (ultimoLadoVisto == 1) {
    enviarSinalPonteH(VELOCIDADE_BUSCA, -VELOCIDADE_BUSCA);
  } else {
    enviarSinalPonteH(-VELOCIDADE_BUSCA, VELOCIDADE_BUSCA);
  }
}

void executarAtaque() {
  if (lCtr) {
    enviarSinalPonteH(VELOCIDADE_ATAQUE, VELOCIDADE_ATAQUE);
  } else if (lEsq) {
    enviarSinalPonteH(VELOCIDADE_CURVA, VELOCIDADE_ATAQUE);
  } else if (lDir) {
    enviarSinalPonteH(VELOCIDADE_ATAQUE, VELOCIDADE_CURVA);
  }
}

void executarFugaDeLinhaE() {
  enviarSinalPonteH(-255, -255);
  delay(280);
  enviarSinalPonteH(255, -255);
  delay(750);
}

void executarFugaDeLinhaD() {
  enviarSinalPonteH(-255, -255);
  delay(280);
  enviarSinalPonteH(-255, 255);
  delay(750);
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
    analogWrite(MOTOR_ESQ1, 0);
    analogWrite(MOTOR_ESQ2, esq);
  } else if (esq < 0) {
    analogWrite(MOTOR_ESQ1, abs(esq));
    analogWrite(MOTOR_ESQ2, 0);
  } else {
    analogWrite(MOTOR_ESQ1, 0);
    analogWrite(MOTOR_ESQ2, 0);
  }

  if (dir > 0) {
    analogWrite(MOTOR_DIR1, 0);
    analogWrite(MOTOR_DIR2, dir);
  } else if (dir < 0) {
    analogWrite(MOTOR_DIR1, abs(dir));
    analogWrite(MOTOR_DIR2, 0);
  } else {
    analogWrite(MOTOR_DIR1, 0);
    analogWrite(MOTOR_DIR2, 0);
  }
}