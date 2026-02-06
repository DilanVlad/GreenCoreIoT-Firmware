#include <DHT.h>

// ==========================================
// CONFIGURACIÓN DE PINES (Según nuestro cableado)
// ==========================================

// --- MESA 1 ---
#define M1_DHT_AMB_PIN  27  // Humedad/Temp Ambiente
#define M1_DHT_CAN_PIN  4   // Humedad Canopia 
#define M1_SOIL_PIN     32  // Humedad Suelo 
#define M1_LDR_PIN      35  // Luz (Analógico)

// --- MESA 2 ---
#define M2_DHT_AMB_PIN  18  // Humedad/Temp Ambiente 
#define M2_DHT_CAN_PIN  19  // Humedad Canopia 
#define M2_SOIL_PIN     33  // Humedad Suelo 
#define M2_LDR_PIN      14  // Luz (Digital)

// ==========================================
// OBJETOS Y VARIABLES
// ==========================================
#define DHTTYPE DHT11

// Creamos 4 objetos independientes para no mezclar lecturas
DHT dht1_amb(M1_DHT_AMB_PIN, DHTTYPE);
DHT dht1_can(M1_DHT_CAN_PIN, DHTTYPE);

DHT dht2_amb(M2_DHT_AMB_PIN, DHTTYPE);
DHT dht2_can(M2_DHT_CAN_PIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=== INICIANDO SISTEMA GREENCORE IOT ===");
  Serial.println("=== MODO PRUEBA DE SENSORES ===");

  // Iniciar los 4 sensores DHT
  dht1_amb.begin();
  dht1_can.begin();
  dht2_amb.begin();
  dht2_can.begin();

  // Configurar el pin digital del LDR de la Mesa 2
  pinMode(M2_LDR_PIN, INPUT);
}

void loop() {
  Serial.println("\n------------------------------------------------");
  
  // ==========================================
  // LECTURA MESA 1
  // ==========================================
  float t1_amb = dht1_amb.readTemperature();
  float h1_amb = dht1_amb.readHumidity();
  float h1_can = dht1_can.readHumidity(); // Solo nos interesa humedad en canopia

  // Lectura Analógica Suelo (0-4095)
  int rawSoil1 = analogRead(M1_SOIL_PIN);
  // Mapeo inverso: 4095 es seco, 1600 es mojado (ajustar según tus pruebas)
  int pctSoil1 = map(rawSoil1, 4095, 1600, 0, 100); 
  pctSoil1 = constrain(pctSoil1, 0, 100);

  // Lectura Analógica Luz (0-4095)
  int rawLdr1 = analogRead(M1_LDR_PIN);
  
  Serial.println("--- [MESA 1] ---");
  printSensor("Ambiente", t1_amb, h1_amb);
  printSimple("Canopia (%)", h1_can);
  Serial.printf("Suelo:      %d%% (Raw: %d)\n", pctSoil1, rawSoil1);
  Serial.printf("Luz (Analog): %d\n", rawLdr1);


  // ==========================================
  // LECTURA MESA 2
  // ==========================================
  float t2_amb = dht2_amb.readTemperature();
  float h2_amb = dht2_amb.readHumidity();
  float h2_can = dht2_can.readHumidity();

  // Lectura Analógica Suelo
  int rawSoil2 = analogRead(M2_SOIL_PIN);
  int pctSoil2 = map(rawSoil2, 4095, 1600, 0, 100);
  pctSoil2 = constrain(pctSoil2, 0, 100);

  // Lectura DIGITAL Luz (1 o 0)
  int estadoLuz2 = digitalRead(M2_LDR_PIN);

  Serial.println("\n--- [MESA 2] ---");
  printSensor("Ambiente", t2_amb, h2_amb);
  printSimple("Canopia (%)", h2_can);
  Serial.printf("Suelo:      %d%% (Raw: %d)\n", pctSoil2, rawSoil2);
  
  // Interpretación del sensor digital
  Serial.print("Luz (Digital): ");
  if(estadoLuz2 == 0) Serial.println("CLARO (0)"); 
  else Serial.println("OSCURO (1)"); 
  // Nota: Si sale al revés, gira el tornillito azul del módulo

  delay(10000); // Esperar 10 segundos entre lecturas
}

// Funciones auxiliares para imprimir bonito
void printSensor(String nombre, float t, float h) {
  if (isnan(t) || isnan(h)) {
    Serial.printf("%s: ERROR LECTURA\n", nombre.c_str());
  } else {
    Serial.printf("%s: %.1f°C | %.1f%%\n", nombre.c_str(), t, h);
  }
}

void printSimple(String nombre, float h) {
  if (isnan(h)) {
    Serial.printf("%s: ERROR\n", nombre.c_str());
  } else {
    Serial.printf("%s: %.1f%%\n", nombre.c_str(), h);
  }
}
