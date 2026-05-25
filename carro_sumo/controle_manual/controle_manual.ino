#include <SoftwareSerial.h>
#include "Adafruit_VL53L1X.h"

// Definições dos pinos para ponte H
#define MOTOR_ESQ1 5
#define MOTOR_ESQ2 6
#define MOTOR_DIR1 10
#define MOTOR_DIR2 9

// Sensores de distância pino XSHUT
#define SENSOR1_XSHUT 4
#define SENSOR2_XSHUT 7
#define SENSOR8_XSHUT 8

// Adafruit_VL53L1X sensorEsq = Adafruit_VL53L1X(SENSOR1_XSHUT);
// Adafruit_VL53L1X sensorCtr = Adafruit_VL53L1X(SENSOR2_XSHUT);
// Adafruit_VL53L1X sensorDir = Adafruit_VL53L1X(SENSOR3_XSHUT);

// Bluetooth nos pinos 11 e 12 (evitando conflito com outros pinos)
SoftwareSerial bluetooth(12, 11); // RX, TX

void setup() {
  bluetooth.begin(9600);
  Serial.begin(115200);
  Wire.begin();

  // inicializarSensoresLaser();
  configurarMotores();
}

void loop() {
  // Controle Bluetooth | ponte H com relés
  if (bluetooth.available()) {
    char comando = bluetooth.read();
    if (comando == 'F') ponteHReles(LOW,LOW);
    else if (comando == 'B') ponteHReles(HIGH, HIGH);
    else if (comando == 'L') ponteHReles(LOW, HIGH);
    else if (comando == 'R') ponteHReles(HIGH,LOW);
    else if (comando == 'S') ponteHReles(3,3);
  }

  // // Monitoramento do Sensor 1
  // if (sensorEsq.dataReady()) {
  //   Serial.print("S1: ");
  //   Serial.print(sensorEsq.distance());
  //   Serial.print("mm | ");
  //   sensorEsq.clearInterrupt();
  // }

  // // Monitoramento do Sensor 2
  // if (sensorCtr.dataReady()) {
  //   Serial.print("S2: ");
  //   Serial.print(sensorCtr.distance());
  //   Serial.print("mm | ");
  //   sensorCtr.clearInterrupt();
  // }

  // // Monitoramento do Sensor 3
  // if (sensorDir.dataReady()) {
  //   Serial.print("S3: ");
  //   Serial.print(sensorDir.distance());
  //   Serial.println("mm");
  //   sensorDir.clearInterrupt();
  // }
}

// void inicializarSensoresLaser() {
//   // Configuração dos pinos XSHUT
//   pinMode(SENSOR1_XSHUT, OUTPUT);
//   pinMode(SENSOR2_XSHUT, OUTPUT);
//   pinMode(SENSOR3_XSHUT, OUTPUT);

//   // Reset dos sensores
//   digitalWrite(SENSOR1_XSHUT, LOW);
//   digitalWrite(SENSOR2_XSHUT, LOW);
//   digitalWrite(SENSOR3_XSHUT, LOW);
//   delay(10);

//   // Inicializa Sensor 1 e muda endereço
//   digitalWrite(SENSOR1_XSHUT, HIGH);
//   delay(10);
//   if (!sensorEsq.begin(0x29, &Wire)) {
//     Serial.println("Erro Sensor 1");
//     while (1)
//       ;
//   }
//   s_dist1.VL53L1X_SetI2CAddress(0x30); //muda endereço para 0x30
//   s_dist1.startRanging();

//   // Inicializa Sensor 2 (fica no endereço padrão 0x29)
//   digitalWrite(SENSOR2_XSHUT, HIGH);
//   delay(10);
//   if (!sensorCtr.begin(0x29, &Wire)) {
//     Serial.println("Erro Sensor 2");
//     while (1)
//       ;
//   }
//   sensorCtr.VL53L1X_SetI2CAddress(0x27); //muda endereço para 0x27
//   sensorCtr.startRanging();

//   // Inicializa Sensor 3 (fica no endereço padrão 0x29)
//   digitalWrite(SENSOR3_XSHUT, HIGH);
//   delay(10);
//   if (!sensorDir.begin(0x29, &Wire)) {
//     Serial.println("Erro Sensor 3");
//     while (1)
//       ;
//   }
//   sensorDir.startRanging();

// }

void ponteHReles(int esq, int dir) {
  // --- CONTROLAR MOTOR ESQUERDO ---
  if (esq == LOW) {
    // Andar para Frente: 
    digitalWrite(MOTOR_ESQ1, HIGH);
    digitalWrite(MOTOR_ESQ2, LOW);
  } 
  else if (esq == HIGH) {
    // Andar para Trás: 
    digitalWrite(MOTOR_ESQ1, LOW);
    digitalWrite(MOTOR_ESQ2, HIGH); 
  } 
  else if(esq == 3) {
    // Parar / Freio: 
    digitalWrite(MOTOR_ESQ1, LOW);
    digitalWrite(MOTOR_ESQ2, LOW);
  }

  // --- CONTROLAR MOTOR DIREITO ---
  if (dir == LOW) {
    // Andar para Frente: 
    digitalWrite(MOTOR_DIR1, HIGH);
    digitalWrite(MOTOR_DIR2, LOW);
  } 
  else if (dir == HIGH) {
    // Andar para Trás:
    digitalWrite(MOTOR_DIR1, LOW);
    digitalWrite(MOTOR_DIR2, HIGH);
  } 
  else if(dir == 3){
    // Parar
    digitalWrite(MOTOR_DIR1, LOW);
    digitalWrite(MOTOR_DIR2, LOW);
  }
}
void configurarMotores() { 
  pinMode(MOTOR_ESQ1, OUTPUT);
  pinMode(MOTOR_ESQ2, OUTPUT);
  pinMode(MOTOR_DIR1, OUTPUT);
  pinMode(MOTOR_DIR2, OUTPUT);
}