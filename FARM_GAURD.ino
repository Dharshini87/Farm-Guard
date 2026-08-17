#define BLYNK_TEMPLATE_ID "TMPL3cbvD9-Hw"
#define BLYNK_TEMPLATE_NAME "Farm Guard"
#define BLYNK_AUTH_TOKEN "kWToogFCGaR2JS_-8_PwtvqXljyDqos9"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// WiFi Credentials
char ssid[] = "Maran";
char pass[] = "maran11187";

// ---------- Pins ----------
#define DHTPIN 4
#define DHTTYPE DHT11

#define SOIL_PIN 34
#define SMOKE_PIN 35
#define FLAME_PIN 27
#define BUZZER_PIN 26
#define GREEN_LED 25

DHT dht(DHTPIN, DHTTYPE);

// ---------- Notification Flags ----------
bool fireSent = false;
bool smokeSent = false;
bool tempSent = false;

// ---------- Fire Detection Stability ----------
int flameLowCount = 0;
bool fireConfirmed = false;

void setup() {

  Serial.begin(115200);

  dht.begin();

  pinMode(FLAME_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(GREEN_LED, LOW);

  Serial.println("===== SMART FARM GUARDIAN =====");

  // Connect to WiFi & Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop() {

  // Run Blynk
  Blynk.run();

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  int soilValue = analogRead(SOIL_PIN);
  int smokeValue = analogRead(SMOKE_PIN);
  int flameValue = digitalRead(FLAME_PIN);

  // Convert Soil Value to Percentage
  int soilPercent = map(soilValue, 4095, 0, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);

  // ---------- Send Sensor Values to Blynk ----------
  Blynk.virtualWrite(V0, soilPercent);
  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, humidity);
  Blynk.virtualWrite(V3, smokeValue);

  bool danger = false;

  // ---------- DHT ----------
  Serial.print("🌡 Temperature : ");
  Serial.print(temperature);
  Serial.println(" °C");

  // Temperature Status
  if (temperature < 35) {
    Serial.println("Temperature Status : NORMAL");
    Blynk.virtualWrite(V7, "Normal");
    tempSent = false;
  }
  else if (temperature <= 45) {
    Serial.println("Temperature Status : HIGH");
    Blynk.virtualWrite(V7, "High");
    tempSent = false;
  }
  else {
    Serial.println("Temperature Status : DANGER");
    Blynk.virtualWrite(V7, "Danger");

    if (!tempSent) {
      Blynk.logEvent("temperature_alert");
      tempSent = true;
    }
  }

  Serial.print("💧 Humidity : ");
  Serial.print(humidity);
  Serial.println(" %");

  // Humidity Status
  if (humidity < 30) {
    Serial.println("Humidity Status : DRY AIR");
    Blynk.virtualWrite(V8, "Dry Air");
  }
  else if (humidity <= 70) {
    Serial.println("Humidity Status : GOOD");
    Blynk.virtualWrite(V8, "Good");
  }
  else {
    Serial.println("Humidity Status : VERY HUMID");
    Blynk.virtualWrite(V8, "Very Humid");
  }

  // ---------- Soil ----------
  Serial.print("🌱 Soil Moisture : ");
  Serial.print(soilPercent);
  Serial.println(" %");

  // Soil Status
  if (soilPercent >= 80) {
    Serial.println("Soil Status : VERY WET");
    Blynk.virtualWrite(V9, "Very Wet");
  }
  else if (soilPercent >= 60) {
    Serial.println("Soil Status : GOOD");
    Blynk.virtualWrite(V9, "Good");
  }
  else if (soilPercent >= 30) {
    Serial.println("Soil Status : DRY");
    Blynk.virtualWrite(V9, "Dry");
  }
  else {
    Serial.println("Soil Status : WATER NEEDED");
    Blynk.virtualWrite(V9, "Water Needed");
  }

  // ---------- Smoke ----------
  Serial.print("💨 Smoke Value : ");
  Serial.println(smokeValue);

  // Smoke Status
  if (smokeValue < 800) {
    Serial.println("Smoke Status : NORMAL");
    Blynk.virtualWrite(V5, "Normal");
    smokeSent = false;
  }
  else if (smokeValue < 1500) {
    Serial.println("Smoke Status : HIGH SMOKE");
    Blynk.virtualWrite(V5, "High Smoke");
    smokeSent = false;
  }
  else {
    Serial.println("Smoke Status : SMOKE DETECTED");
    Blynk.virtualWrite(V5, "Smoke Detected");
    danger = true;

    if (!smokeSent) {
      Blynk.logEvent("smoke_alert");
      smokeSent = true;
    }
  }

  // ---------- Flame ----------
  // Your sensor:
  // 1 = No Fire
  // 0 = Fire

  if (flameValue == LOW) {

    // Count consecutive LOW readings
    flameLowCount++;

    Serial.print("🔥 Flame sensor LOW - confirmation ");
    Serial.print(flameLowCount);
    Serial.println("/1");

    // Confirm fire only after 2 consecutive LOW readings
    if (flameLowCount >= 1) {

      fireConfirmed = true;

      Serial.println("🔥 FIRE CONFIRMED!");

      Blynk.virtualWrite(V6, "Fire Detected");

      danger = true;

      if (!fireSent) {
        Blynk.logEvent("fire_alert");
        fireSent = true;
      }
    }

  }
  else {

    // Sensor is HIGH = no fire
    flameLowCount = 0;
    fireConfirmed = false;

    Serial.println("🔥 Fire Status : SAFE");

    Blynk.virtualWrite(V6, "SAFE");

    fireSent = false;
  }

  // ---------- High Temperature ----------
  if (temperature > 45) {
    Serial.println("🌡 High Temperature!");
    danger = true;
  }

  // ---------- Fire Alert LED on Blynk ----------
  if (danger) {
    Blynk.virtualWrite(V4, 1);
  }
  else {
    Blynk.virtualWrite(V4, 0);
  }

  // ---------- Buzzer & Green LED ----------
  if (danger) {

    // Buzzer ON
    digitalWrite(BUZZER_PIN, HIGH);

    // Green LED OFF during danger
    digitalWrite(GREEN_LED, LOW);

    delay(500);

  }
  else {

    // Buzzer OFF
    digitalWrite(BUZZER_PIN, LOW);

    // Green LED Blinks in normal condition
    digitalWrite(GREEN_LED, HIGH);
    delay(500);

    digitalWrite(GREEN_LED, LOW);
    delay(500);
  }

  Serial.println("----------------------------");
  delay(5000);
}