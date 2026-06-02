#include "Adafruit_VL53L1X.h"

#define PIN_LINHA_FRONT_ESQ 2
#define PIN_LINHA_FRONT_DIR 12

#define MOTOR_ESQ1 5
#define MOTOR_ESQ2 6
#define MOTOR_DIR1 9
#define MOTOR_DIR2 10

#define LED_ALERTA 13

#define SENSOR1_XSHUT 7
#define SENSOR2_XSHUT 11
#define SENSOR3_XSHUT 8

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
  Serial.begin(9600);
  Wire.begin();

  pinMode(PIN_LINHA_FRONT_DIR, INPUT);
  pinMode(PIN_LINHA_FRONT_ESQ, INPUT);

  inicializarSensoresLaser();
  Serial.println("Serial iniciado");
}

void loop() {
  verificarBordaDaArena();
  atualizarEDetectouInimigo();

}

bool verificarBordaDaArena() {
  Serial.print("LE: ");
  Serial.print(digitalRead(PIN_LINHA_FRONT_ESQ));
  Serial.print(" | LD: ");
  Serial.print(digitalRead(PIN_LINHA_FRONT_DIR));
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

void atualizarEDetectouInimigo() {
  distLaserEsq = sensorEsq.distance();
  distLaserCtr = sensorCtr.distance();
  distLaserDir = sensorDir.distance();

  lEsq = (distLaserEsq > distMin && distLaserEsq < distMax);
  lCtr = (distLaserCtr > distMin && distLaserCtr < distMax);
  lDir = (distLaserDir > distMin && distLaserDir < distMax);

  Serial.print(" E :");
  Serial.print(lEsq);
  Serial.print(" | ");
  Serial.print("C :");
  Serial.print(lCtr);
  Serial.print(" | ");
  Serial.print("D :");
  Serial.println(lDir);

}