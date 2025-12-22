#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// --- Configurações de Rede e Firebase ---
#define WIFI_SSID "SEU_WIFI_NOME"
#define WIFI_PASSWORD "SUA_SENHA_WIFI"
#define API_KEY "AIzaSyB830NJZsrcvGvKo-_2i8QgAfptRDKRdLM"
#define DATABASE_URL "https://gen-lang-client-0335555331-default-rtdb.firebaseio.com/" 

// --- Definição de Pinos ---
#define SS_PIN    21
#define RST_PIN   22
#define RELE_PIN  4    // Solenoide
#define BUZZER    12   // Módulo Buzzer Ativo
#define SENSOR_IR 13   // Sensor de Mão

MFRC522 rfid(SS_PIN, RST_PIN);
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// --- Configurações ---
String CARTOES_LIBERADOS[] = {"A1 B2 C3 D4"}; 
const int TEMPO_ESPERA_MAO = 10000; // Timeout de 10 segundos conforme solicitado

// --- Protótipos das Funções ---
bool verificarAcesso(String id);
void processarAcessoValido(String id);
void liberarDispenser();
void beepCurto();
void beepLongo();
void connectWiFi();
void sendDataToCloud(String id, String status);

void setup() {
  Serial.begin(115200);
  
  // Inicializa Hardware
  SPI.begin();
  rfid.PCD_Init();
  pinMode(RELE_PIN, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(SENSOR_IR, INPUT);
  
  digitalWrite(RELE_PIN, HIGH); // Trava garantida no início
  digitalWrite(BUZZER, LOW);    

  // Conecta WiFi e Firebase
  connectWiFi();
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
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
    processarAcessoValido(idAtual);
  } else {
    Serial.println("Cartão Inválido: " + idAtual);
    // Opcional: Logar tentativas inválidas também
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

void processarAcessoValido(String id) {
  beepCurto(); 

  unsigned long inicioMilis = millis();
  bool maoDetectada = false;

  // Loop de 10 segundos esperando a mão
  while (millis() - inicioMilis < TEMPO_ESPERA_MAO) {
    if (digitalRead(SENSOR_IR) == LOW) { // Mão detectada
      maoDetectada = true;
      break;
    }
    
    // Bips de orientação
    digitalWrite(BUZZER, HIGH);
    delay(30);
    digitalWrite(BUZZER, LOW);
    delay(470); 
  }

  if (maoDetectada) {
    sendDataToCloud(id, "Sucesso");
    liberarDispenser();
  } else {
    sendDataToCloud(id, "Timeout");
    Serial.println("TIMEOUT: Funcionário não posicionou a mão.");
    digitalWrite(BUZZER, HIGH);
    delay(3000); 
    digitalWrite(BUZZER, LOW);
  }
}

void liberarDispenser() {
  beepLongo(); 
  digitalWrite(RELE_PIN, LOW); // Destrava
  delay(5000);                 
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

void connectWiFi() {
  Serial.print("Conectando WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Conectado: ");
  Serial.println(WiFi.localIP());
}

void sendDataToCloud(String id, String status) {
  if (Firebase.ready()) {
    FirebaseJson json;
    json.set("id_funcionario", id);
    json.set("status_uso", status);
    // OBS: O Timestamp idealmente vem de um servidor NTP, mas para simplificar:
    // json.set("timestamp", millis()); 
    
    // Push adiciona um novo nó na lista 'logs'
    if (Firebase.pushJSON(fbdo, "/logs", json)) {
      Serial.println("Dado enviado para o Firebase!");
    } else {
      Serial.println("Erro no envio: " + fbdo.errorReason());
    }
  }
}