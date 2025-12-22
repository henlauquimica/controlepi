#include <SPI.h>
#include <MFRC522.h>

// --- Definição de Pinos ---
#define SS_PIN    21
#define RST_PIN   22
#define RELE_PIN  4    // Solenoide
#define BUZZER    12   // Módulo Buzzer Ativo
#define SENSOR_IR 13   // Sensor de Mão

MFRC522 rfid(SS_PIN, RST_PIN);

// --- Configurações ---
String CARTOES_LIBERADOS[] = {"A1 B2 C3 D4"}; 
const int TEMPO_ESPERA_MAO = 10000; // Timeout de 10 segundos conforme solicitado

// --- Protótipos das Funções ---
bool verificarAcesso(String id);
void processarAcessoValido();
void liberarDispenser();
void beepCurto();
void beepLongo();


void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  
  pinMode(RELE_PIN, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(SENSOR_IR, INPUT);
  
  digitalWrite(RELE_PIN, HIGH); // Trava garantida no início
  digitalWrite(BUZZER, LOW);    
  
  Serial.println("SISTEMA PRONTO - Aguardando Identificação");
}

void loop() {
  // Busca por cartões
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  String idAtual = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    idAtual += (rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    idAtual += String(rfid.uid.uidByte[i], HEX);
  }
  idAtual.toUpperCase();
  idAtual.trim();

  // Verifica se o cartão é autorizado
  if (verificarAcesso(idAtual)) {
    Serial.println("Acesso Autorizado: " + idAtual);
    processarAcessoValido();
  } else {
    // Se o cartão for inválido, não faz nada (silêncio), apenas log no Serial
    Serial.println("Cartão Inválido: " + idAtual);
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

bool verificarAcesso(String id) {
  for (String autorizado : CARTOES_LIBERADOS) {
    if (id == autorizado) return true;
  }
  return false;
}

void processarAcessoValido() {
  beepCurto(); // Som de "OK, cartão lido"

  unsigned long inicioMilis = millis();
  bool maoDetectada = false;

  // Loop de 10 segundos esperando a mão
  while (millis() - inicioMilis < TEMPO_ESPERA_MAO) {
    if (digitalRead(SENSOR_IR) == LOW) { // Mão detectada
      maoDetectada = true;
      break;
    }
    
    // Bips curtos de orientação (o usuário sabe que o sistema está esperando)
    digitalWrite(BUZZER, HIGH);
    delay(30);
    digitalWrite(BUZZER, LOW);
    delay(470); // Bipa a cada meio segundo
  }

  if (maoDetectada) {
    liberarDispenser();
  } else {
    // CASO ESGOTE O TIMEOUT: Alarme contínuo de 3 segundos (Erro de Procedimento)
    Serial.println("TIMEOUT: Funcionário não posicionou a mão.");
    digitalWrite(BUZZER, HIGH);
    delay(3000); 
    digitalWrite(BUZZER, LOW);
  }
}

void liberarDispenser() {
  beepLongo(); // Som de "Pode retirar o produto"
  digitalWrite(RELE_PIN, LOW); // Destrava o solenoide
  delay(5000);                 // Tempo aberto
  digitalWrite(RELE_PIN, HIGH); // Tranca
}

void beepCurto() {
  digitalWrite(BUZZER, HIGH);
  delay(100);
  digitalWrite(BUZZER, LOW);
}

void beepLongo() {
  digitalWrite(BUZZER, HIGH);
  delay(600);
  digitalWrite(BUZZER, LOW);
}