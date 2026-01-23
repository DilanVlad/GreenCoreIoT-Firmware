#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// ================== CONFIGURACIÓN WIFI ==================
const char* ssid = "INNO-";
const char* password = "pass";

// ================== API ==================
const char* serverUrl = "https://bq1skqgh-7278.use2.devtunnels.ms/api/Lecturas";

// ================== SLOTS FIJOS ==================
#define SLOT_1 1
#define SLOT_2 2



// ================== PINES SLOT 1 ==================
#define DHTPIN_1 27
#define SOIL_PIN_1 34
#define LDR_PIN_1 35

// ================== HARDWARE ==================
#define DHTTYPE DHT11
DHT dht1(DHTPIN_1, DHTTYPE);

// ======================================================

void setup() {
  Serial.begin(115200);
  dht1.begin();

  // Conexión WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ WiFi conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// ======================================================

void loop() {

  // ==========================================
  // ===== SLOT 1 (SENSORES REALES) ===========
  // ==========================================
  float temp1 = dht1.readTemperature();
  float humAmb1 = dht1.readHumidity();

  // Verificamos que el DHT lea correctamente antes de enviar nada
  if (!isnan(temp1) && !isnan(humAmb1)) {

    // --- Procesamiento Suelo ---
    int rawSoil1 = analogRead(SOIL_PIN_1);
    int humSoil1 = map(rawSoil1, 4095, 1600, 0, 100);
    humSoil1 = constrain(humSoil1, 0, 100);

    // --- Procesamiento Luz ---
    int rawLdr1 = analogRead(LDR_PIN_1);
    int luz1 = map(rawLdr1, 4095, 200, 0, 10000);
    luz1 = constrain(luz1, 0, 10000);

    // --- Procesamiento Canopia (Simulado basado en ambiente) ---
    float humCanopia1 = humAmb1 + 2.0; 

    // --- ENVÍO CON LOS ID CORRECTOS ---
    enviarLectura(SLOT_1, "TEMP", temp1);       // Temperatura
    enviarLectura(SLOT_1, "HUMA", humAmb1);     // Humedad Ambiental
    enviarLectura(SLOT_1, "HUMS", humSoil1);    // Humedad Sustrato
    enviarLectura(SLOT_1, "LUX", luz1);         // Luminosidad
    enviarLectura(SLOT_1, "HUMC", humCanopia1); // Humedad Canopia
  } else {
    Serial.println("⚠️ Error leyendo DHT11 en Slot 1");
  }

  // ==========================================
  // ===== SLOT 2 (DATOS SIMULADOS) ===========
  // ==========================================
  // Simulamos variaciones leves respecto al Slot 1
  float temp2 = temp1 + 1.5;
  float humAmb2 = humAmb1 - 3.0;
  
  // OJO: He corregido los IDs aquí también para que coincidan con los del Slot 1
  enviarLectura(SLOT_2, "TEMP", temp2);
  enviarLectura(SLOT_2, "HUMA", humAmb2);       // Antes era HUM_AMB (Corregido)
  enviarLectura(SLOT_2, "HUMC", humAmb2 + 1.2); // Antes era HUM_CAN (Corregido)
  
  // Nota: Si necesitas simular HUMS y LUX para el Slot 2, agrégalos aquí.

  Serial.println("------------------------------------------------");
  delay(10000); // Espera 10 segundos antes de la siguiente ronda
}

// ======================================================

void enviarLectura(int slot, String idVar, float valor) {

  if (WiFi.status() != WL_CONNECTED) {
    revealError("WiFi desconectado");
    return;
  }

  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");

  // Construcción del JSON
  String json = "{";
  json += "\"slot\":" + String(slot) + ",";
  json += "\"idVar\":\"" + idVar + "\",";
  json += "\"valorLec\":" + String(valor, 2);
  json += "}";

  Serial.print("📤 Enviando [Slot " + String(slot) + " - " + idVar + "]: ");

  int httpCode = http.POST(json);

  if (httpCode > 0) {
    Serial.printf("✅ Éxito (HTTP %d)\n", httpCode);
  } else {
    Serial.printf("❌ Error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}

void revealError(String msg) {
  Serial.println("❌ " + msg);
}