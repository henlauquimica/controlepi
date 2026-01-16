#include <HTTPClient.h>
#include <MFRC522.h>
#include <SPI.h>
#include <WiFi.h>

// --- Configurações de Rede e Firebase ---
#define WIFI_SSID "SEU_WIFI_NOME"
#define WIFI_PASSWORD "SUA_SENHA_WIFI"

// --- Supabase ---
#define SUPABASE_URL "https://inwskxdquwfhhryxpghh.supabase.co"
#define SUPABASE_ANON_KEY "sb_publishable_gQTZhkfL1mcJtrVN_uuIrw_N6UJp82F"

// --- Definição de Pinos ---
#define SS_PIN 21
#define RST_PIN 22
#define RELE_PIN 4   // Solenoide
#define BUZZER 12    // Módulo Buzzer Ativo
#define SENSOR_IR 13 // Sensor de Mão

MFRC522 rfid(SS_PIN, RST_PIN);

// --- Configurações ---
String CARTOES_LIBERADOS[] = {"A1 B2 C3 D4"};
const int TEMPO_ESPERA_MAO =
    10000; // Timeout de 10 segundos conforme solicitado

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

  // Conecta WiFi
  connectWiFi();

  Serial.println("SISTEMA PRONTO - Aguardando Identificação");
}

void loop() {
  // Busca por cartões
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    return;

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
    if (id == autorizado)
      return true;
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
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wifi desconectado!");
    return;
  }

  HTTPClient http;

  // Endpoint da tabela 'logs'
  String url = String(SUPABASE_URL) + "/rest/v1/logs";

  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", SUPABASE_ANON_KEY);
  http.addHeader("Authorization", "Bearer " + String(SUPABASE_ANON_KEY));
  http.addHeader("Prefer", "return-minimal");

  String json =
      "{\"id_funcionario\":\"" + id + "\",\"status_uso\":\"" + status + "\"}";

  int httpCode = http.POST(json);

  if (httpCode == 201) {
    Serial.println("Dado enviado para o Supabase!");
  } else {
    Serial.print("Erro no envio. Código: ");
    Serial.println(httpCode);
    Serial.println(http.getString());
  }

  http.end();
}