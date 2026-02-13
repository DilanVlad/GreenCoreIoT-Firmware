#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// ================== CONFIGURACIÓN WIFI ==================
const char* ssid = "GreenCore";     
const char* password = "green1234";

// ================== ENDPOINT ==================
const char* serverUrl = "https://bq1skqgh-7043.use2.devtunnels.ms/api/Lecturas";

// ================== SLOTS ==================
#define SLOT_1 1   // MES-001 (Mesa 1)
#define SLOT_2 2   // MES-002 (Mesa 2)

// ================== PINES  ==================
// --- MESA 1 ---
#define M1_DHT_AMB_PIN  27  // Humedad/Temp Ambiente
#define M1_DHT_CAN_PIN  4   // Humedad Canopia (Cable Amarillo)
#define M1_SOIL_PIN     32  // Humedad Suelo (Analógico)
#define M1_LDR_PIN      35  // Luz (Analógico)

// --- MESA 2 ---
#define M2_DHT_AMB_PIN  18  // Humedad/Temp Ambiente (Cable Blanco)
#define M2_DHT_CAN_PIN  19  // Humedad Canopia (Cable Blanco)
#define M2_SOIL_PIN     33  // Humedad Suelo (Cable Naranja)
#define M2_LDR_PIN      14  // Luz (Digital - Cable Azul)

// ================== HARDWARE ==================
#define DHTTYPE DHT11

// Objetos independientes para cada sensor DHT
DHT dht1_amb(M1_DHT_AMB_PIN, DHTTYPE);
DHT dht1_can(M1_DHT_CAN_PIN, DHTTYPE);
DHT dht2_amb(M2_DHT_AMB_PIN, DHTTYPE);
DHT dht2_can(M2_DHT_CAN_PIN, DHTTYPE);

// ======================================================

void setup() {
  Serial.begin(115200);
  
  // Iniciar Sensores
  dht1_amb.begin();
  dht1_can.begin();
  dht2_amb.begin();
  dht2_can.begin();
  
  pinMode(M2_LDR_PIN, INPUT); // El LDR de la Mesa 2 es digital

  // Conexión WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// ======================================================

void loop() {
  Serial.println("\n------------------------------------------------");
  
  if (WiFi.status() == WL_CONNECTED) {
    procesarMesa1();
    procesarMesa2();
  } else {
    Serial.println("Error: WiFi desconectado. Intentando reconectar...");
    WiFi.reconnect();
  }

  Serial.println("Esperando 10 segundos para la siguiente lectura...");
  delay(10000); // 10 Segundos de espera
}

// ======================================================
//  MESA 1
// ======================================================
void procesarMesa1() {
  Serial.println("--- PROCESANDO MESA 1 ---");

  // 1. Lectura Sensores
  float temp = dht1_amb.readTemperature();
  float humAmb = dht1_amb.readHumidity();
  float humCan = dht1_can.readHumidity();
  
  int rawSoil = analogRead(M1_SOIL_PIN);
  int humSoil = map(rawSoil, 4095, 1600, 0, 100); // Calibrar el 1600 con agua real
  humSoil = constrain(humSoil, 0, 100);

  int rawLdr = analogRead(M1_LDR_PIN);
  
  // === CAMBIO REALIZADO AQUI ===
  // Antes: map(rawLdr, 4095, 0, 0, 10000);
  // Ahora: map(rawLdr, 0, 4095, 0, 10000);
  // Lógica: 0 (Bajo) = 0 (Oscuro). 4095 (Alto) = 10000 (Claro).
  int luz = map(rawLdr, 0, 4095, 0, 10000); 
  luz = constrain(luz, 0, 10000);

  // 2. Validación y Envío
  if (isnan(temp) || isnan(humAmb)) {
    Serial.println("Error leyendo DHT11 Ambiente Mesa 1");
  } else {
    enviarLectura(SLOT_1, "TEMP", temp);
    enviarLectura(SLOT_1, "HUMA", humAmb);
  }
  
  if (isnan(humCan)) {
     Serial.println("Error leyendo DHT11 Canopia Mesa 1");
  } else {
     enviarLectura(SLOT_1, "HUMC", humCan);
  }

  enviarLectura(SLOT_1, "HUMS", humSoil);
  enviarLectura(SLOT_1, "LUX", luz);
}

// ======================================================
// LÓGICA MESA 2
// ======================================================
void procesarMesa2() {
  Serial.println("--- PROCESANDO MESA 2 ---");

  // 1. Lectura Sensores
  float temp = dht2_amb.readTemperature();
  float humAmb = dht2_amb.readHumidity();
  float humCan = dht2_can.readHumidity();
  
  int rawSoil = analogRead(M2_SOIL_PIN);
  int humSoil = map(rawSoil, 4095, 1600, 0, 100);
  humSoil = constrain(humSoil, 0, 100);

  // Lógica Digital para Luz Mesa 2
  int estadoLuz = digitalRead(M2_LDR_PIN); 
  
  // Si tu sensor digital envía 0 cuando hay LUZ:
  // 0 = Claro (Valor alto 1000)
  // 1 = Oscuro (Valor bajo 0)
  int luz = (estadoLuz == 0) ? 1000 : 0; 
  
  // NOTA: Si ves que la Mesa 2 también va al revés, cambia la linea de arriba por:
  // int luz = (estadoLuz == 1) ? 1000 : 0; 

  // 2. Validación y Envío
  if (isnan(temp) || isnan(humAmb)) {
    Serial.println("Error leyendo DHT11 Ambiente Mesa 2");
  } else {
    enviarLectura(SLOT_2, "TEMP", temp);
    enviarLectura(SLOT_2, "HUMA", humAmb);
  }

  if (isnan(humCan)) {
     Serial.println("Error leyendo DHT11 Canopia Mesa 2");
  } else {
     enviarLectura(SLOT_2, "HUMC", humCan);
  }

  enviarLectura(SLOT_2, "HUMS", humSoil);
  enviarLectura(SLOT_2, "LUX", luz);
}

// ======================================================
// FUNCIÓN DE ENVÍO HTTP
// ======================================================
void enviarLectura(int slot, String idVar, float valor) {
  
  HTTPClient http;
  http.begin(serverUrl);
  http.setTimeout(5000); 
  http.addHeader("Content-Type", "application/json");

  // Crear JSON
  String json = "{";
  json += "\"slot\":" + String(slot) + ",";
  json += "\"idVar\":\"" + idVar + "\",";
  json += "\"valorLec\":" + String(valor, 2);
  json += "}";

  Serial.print(">> Enviando [Mesa ");
  Serial.print(slot);
  Serial.print(" - ");
  Serial.print(idVar);
  Serial.print("]: ");

  int httpCode = http.POST(json);

  if (httpCode > 0) {
    Serial.printf("OK (HTTP %d)\n", httpCode);
  } else { 
    Serial.printf("ERROR: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}
