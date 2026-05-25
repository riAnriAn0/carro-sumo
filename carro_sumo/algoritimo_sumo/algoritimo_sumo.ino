#include <Wire.h>
#include "Adafruit_VL53L1X.h"

// ==========================================
// 1. CONFIGURAÇÕES, PINOS E VARIÁVEIS GLOBAIS
// ==========================================
#define PIN_LINHA_FRONT_ESQ 2
#define PIN_LINHA_FRONT_DIR 3

#define MOTOR_ESQ1 5
#define MOTOR_ESQ2 6
#define MOTOR_DIR1 10
#define MOTOR_DIR2 9

enum Estados { INICIAL,
               PROCURA,
               ATAQUE,
               FUGA_LINHA };
Estados estadoAtual = INICIAL;


//Distância minima e maxima detectada (em mm)
int distMin = 50;
int distMax = 800;


// Sensores de distância pino XSHUT
#define SENSOR1_XSHUT 4
#define SENSOR2_XSHUT 7
#define SENSOR8_XSHUT 8

Adafruit_VL53L1X sensorEsq = Adafruit_VL53L1X(SENSOR1_XSHUT);
Adafruit_VL53L1X sensorCtr = Adafruit_VL53L1X(SENSOR2_XSHUT);
Adafruit_VL53L1X sensorDir = Adafruit_VL53L1X(SENSOR3_XSHUT);

// ==========================================
// CONFIGURAÇÃO INICIAL (SETUP)
// ==========================================
void setup() {
  Serial.begin(115200);
  Wire.begin();

  inicializarSensoresLaser(); 
  configurarMotores();

  estadoAtual = INICIAL;
}

// ==========================================
// LOOP PRINCIPAL (Organizado por fluxo)
// ==========================================
void loop() {
  // Chamada constante da leitura de segurança (Linha Branca)
  if (verificarBordaDaArena()) {
    estadoAtual = FUGA_LINHA;
  }

  // Máquina de Estados
  switch (estadoAtual) {
    case INICIAL:
      executarEstrategiaInicial();  // Aguarda os 5 segundos e faz o movimento inicial
      estadoAtual = PROCURA;
      break;

    case PROCURA:
      executarProcura();
      if (detectouInimigo()) {
        estadoAtual = ATAQUE;
      }
      break;

    case ATAQUE:
      executarAtaque();
      if (!detectouInimigo()) {
        estadoAtual = PROCURA;  // Se o inimigo sumir, volta a procurar
      }
      break;

    case FUGA_LINHA:
      executarFugaDeLinha();
      estadoAtual = PROCURA;  // Após se salvar, volta a procurar
      break;
  }
}
// ==========================================
// 2. FUNÇÕES DA CAMADA DE SENSORES
// ==========================================
bool verificarBordaDaArena() {
  // Retorna verdadeiro imediatamente se um dos sensores de linha ler preto
  if (digitalRead(PIN_LINHA_FRONTA_ESQ) == 0 || digitalRead(PIN_LINHA_FRONT_DIR)  == 0) {
    return true;
  }
  return false;
}
bool detectouInimigo() {
  uint16_t distEsq = sensorEsq.distance();
  uint16_t distCtr = sensorCtr.distance();
  uint16_t distDir = sensorDir.distance();
  if ((distEsq > distMin && distEsq < distMax) || (distDir > distMin && distDir < distMax) || (distCtr > distMin && distCtr < distMax)) {
    return true;
  }
  return false;
}
// ==========================================
// 3. FUNÇÕES DE COMPORTAMENTO
// ==========================================
void executarProcura() {
}

void executarAtaque() {
  // Inimigo focado: despeja força máxima (255) usando a rampa protetora
  controlarMotoresComRampa(255, 255);
}

void executarFugaDeLinha() {
  // Freia bruscamente, recua e gira para o lado oposto da linha
  enviarSinalPonteH(-255, -255);  // Tranco imediato para trás (ignora rampa para emergência)
  delay(300);
  enviarSinalPonteH(255, -255);  // Gira para sair da borda
  delay(250);
  velocidadeAtualEsq = 0;  // Reseta as variáveis da rampa
  velocidadeAtualDir = 0;
}

void executarEstrategiaInicial() {
  delay(5000); /* Aguarda tempo regulamentar */
}
//////////////////////////////// FINALIZADO
void inicializarSensoresLaser() {
  // Configuração dos pinos XSHUT
  pinMode(SENSOR1_XSHUT, OUTPUT);
  pinMode(SENSOR2_XSHUT, OUTPUT);
  pinMode(SENSOR3_XSHUT, OUTPUT);

  // Reset dos sensores
  digitalWrite(SENSOR1_XSHUT, LOW);
  digitalWrite(SENSOR2_XSHUT, LOW);
  digitalWrite(SENSOR3_XSHUT, LOW);
  delay(10);

  // Inicializa Sensor 1 e muda endereço
  digitalWrite(SENSOR1_XSHUT, HIGH);
  delay(10);
  if (!sensorEsq.begin(0x29, &Wire)) {
    Serial.println("Erro Sensor 1");
    while (1)
      ;
  }
  s_dist1.VL53L1X_SetI2CAddress(0x30); //muda endereço para 0x30
  s_dist1.startRanging();

  // Inicializa Sensor 2 (fica no endereço padrão 0x29)
  digitalWrite(SENSOR2_XSHUT, HIGH);
  delay(10);
  if (!sensorCtr.begin(0x29, &Wire)) {
    Serial.println("Erro Sensor 2");
    while (1)
      ;
  }
  sensorCtr.VL53L1X_SetI2CAddress(0x27); //muda endereço para 0x27
  sensorCtr.startRanging();

  // Inicializa Sensor 3 (fica no endereço padrão 0x29)
  digitalWrite(SENSOR3_XSHUT, HIGH);
  delay(10);
  if (!sensorDir.begin(0x29, &Wire)) {
    Serial.println("Erro Sensor 3");
    while (1)
      ;
  }
  sensorDir.startRanging();

}
//////////////////////////////// FINALIZADO
void configurarMotores() { 
  pinMode(MOTOR_ESQ1, OUTPUT);
  pinMode(MOTOR_ESQ2, OUTPUT);
  pinMode(MOTOR_DIR1, OUTPUT);
  pinMode(MOTOR_DIR2, OUTPUT);
}
///////////////////////////////// FINALIZADO
void enviarSinalPonteH(int esq, int dir) {
  // --- CONTROLAR MOTOR ESQUERDO ---
  if (esq > LOW) {
    // Andar para Frente: 
    analogWrite(MOTOR_ESQ1, HIGH);
    analogWrite(MOTOR_ESQ2, LOW);
  } 
  else if (esq < LOW) {
    // Andar para Trás: 
    analogWrite(MOTOR_ESQ1, LOW);
    analogWrite(MOTOR_ESQ2, HIGH); 
  } 
  else {
    // Parar / Freio: 
    analogWrite(MOTOR_ESQ1, LOW);
    analogWrite(MOTOR_ESQ2, LOW);
  }

  // --- CONTROLAR MOTOR ESQUERDO ---
  if (dir > LOW) {
    // Andar para Frente: 
    analogWrite(MOTOR_DIR1, HIGH);
    analogWrite(MOTOR_DIR2, LOW);
  } 
  else if (dir < LOW) {
    // Andar para Trás:
    analogWrite(MOTOR_DIR1, LOW);
    analogWrite(MOTOR_DIR2, HIGH);
  } 
  else {
    // Parar
    analogWrite(MOTOR_DIR1, LOW);
    analogWrite(MOTOR_DIR2, LOW);
  }
}
