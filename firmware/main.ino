/*
  HydroNexus Automated NFT Hydroponic System
  Board: ESP32 DevKit

  The sketch reads pH, TDS, BME680, and ultrasonic water level sensors,
  displays a compact status screen on the OLED, controls four relays, and
  posts a JSON snapshot to the PHP API.

  IMPORTANT: Relay modules are commonly active LOW. Change RELAY_ON and
  RELAY_OFF below if your module is active HIGH.
*/

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_BME680.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

// --------------------------- Network settings ----------------------------
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* API_URL = "https://your-domain.example/dashboard/api.php";
const char* DEVICE_ID = "hydronexus-esp32-01";

// ---------------------------- Pin assignments ----------------------------
const int PH_PIN = 34;          // Analog pH sensor
const int TDS_PIN = 35;         // Analog TDS sensor
const int TRIG_PIN = 5;         // HC-SR04 trigger
const int ECHO_PIN = 18;        // HC-SR04 echo
const int RELAY_PH_UP = 16;     // Relay 1: pH dosing pump
const int RELAY_WATER = 17;     // Relay 2: water pump
const int RELAY_FAN = 19;       // Relay 3: climate fan
const int RELAY_LIGHTS = 23;    // Relay 4: grow lights

// I2C defaults are GPIO 21 (SDA) and GPIO 22 (SCL).
const int OLED_WIDTH = 128;
const int OLED_HEIGHT = 64;
const uint8_t OLED_ADDRESS = 0x3C;

// ------------------------------ Thresholds -------------------------------
const float PH_MIN = 5.5;
const float PH_MAX = 6.5;
const float TEMP_FAN_ON = 35.0;
const float TEMP_FAN_OFF = 33.0;        // Hysteresis avoids relay chatter.
const float LOW_WATER_LEVEL = 35.0;     // Percent.
const float WATER_PUMP_ON_LEVEL = 42.0;
const float WATER_PUMP_OFF_LEVEL = 86.0;

// Most 4-channel relay boards use LOW to energize a channel.
const int RELAY_ON = LOW;
const int RELAY_OFF = HIGH;

Adafruit_BME680 bme;
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

struct SensorReadings {
  float temperature = 0;
  float humidity = 0;
  float pressure = 0;
  float gasResistance = 0;
  float ph = 7.0;
  float tds = 0;
  float waterLevel = 0;
};

SensorReadings readings;
bool bmeReady = false;
bool oledReady = false;
bool fanRunning = false;
bool waterPumpRunning = false;
unsigned long lastPostAt = 0;
const unsigned long POST_INTERVAL_MS = 5000;

void setRelay(int pin, bool shouldRun) {
  digitalWrite(pin, shouldRun ? RELAY_ON : RELAY_OFF);
}

void connectToWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("Wi-Fi connected. IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWi-Fi connection timed out. The system will keep controlling locally.");
  }
}

float readPH() {
  // Calibrate this conversion with pH 4.00 and pH 7.00 buffer solutions.
  const int raw = analogRead(PH_PIN);
  const float voltage = (raw / 4095.0) * 3.3;
  const float calibratedPH = 7.0 + ((2.50 - voltage) / 0.18);
  return constrain(calibratedPH, 0.0, 14.0);
}

float readTDS() {
  // This is a starter approximation. Calibrate with a known TDS solution.
  const int raw = analogRead(TDS_PIN);
  const float voltage = (raw / 4095.0) * 3.3;
  const float tds = (133.42 * voltage * voltage * voltage
                   - 255.86 * voltage * voltage
                   + 857.39 * voltage) * 0.5;
  return max(0.0f, tds);
}

float readWaterLevelPercent() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  const unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) {
    return readings.waterLevel; // Keep the last good value when no echo arrives.
  }

  const float distanceCm = duration * 0.0343 / 2.0;
  const float tankEmptyDistance = 35.0; // Measure your tank and update this.
  const float tankFullDistance = 5.0;
  const float percent = 100.0 * (tankEmptyDistance - distanceCm)
                      / (tankEmptyDistance - tankFullDistance);
  return constrain(percent, 0.0, 100.0);
}

void readSensors() {
  if (bmeReady && bme.performReading()) {
    readings.temperature = bme.temperature;
    readings.humidity = bme.humidity;
    readings.pressure = bme.pressure / 100.0;
    readings.gasResistance = bme.gas_resistance / 1000.0;
  }

  readings.ph = readPH();
  readings.tds = readTDS();
  readings.waterLevel = readWaterLevelPercent();
}

void regulateRelays() {
  // Fan rule: turn on above 35°C and remain on until it cools below 33°C.
  if (!fanRunning && readings.temperature > TEMP_FAN_ON) fanRunning = true;
  if (fanRunning && readings.temperature < TEMP_FAN_OFF) fanRunning = false;
  setRelay(RELAY_FAN, fanRunning);

  // Water pump rule: refill when low and stop when the reservoir is nearly full.
  if (!waterPumpRunning && readings.waterLevel < WATER_PUMP_ON_LEVEL) waterPumpRunning = true;
  if (waterPumpRunning && readings.waterLevel > WATER_PUMP_OFF_LEVEL) waterPumpRunning = false;
  setRelay(RELAY_WATER, waterPumpRunning);

  // pH dosing is intentionally short and conservative. A real installation
  // should use a timed dosing state machine and a calibration routine.
  const bool phNeedsUp = readings.ph < PH_MIN;
  const bool phNeedsDown = readings.ph > PH_MAX;
  setRelay(RELAY_PH_UP, phNeedsUp && !phNeedsDown);

  // Grow lights run continuously in this starter configuration. Replace this
  // with a schedule or a light sensor for production use.
  setRelay(RELAY_LIGHTS, true);
}

void drawOLED() {
  if (!oledReady) return;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("HYDRONEXUS  |  NFT");
  display.drawLine(0, 11, 127, 11, SSD1306_WHITE);

  display.setCursor(0, 17);
  display.printf("T %4.1f C  H %3.0f %%", readings.temperature, readings.humidity);
  display.setCursor(0, 29);
  display.printf("pH %4.2f   TDS %4.0f", readings.ph, readings.tds);
  display.setCursor(0, 41);
  display.printf("Water %3.0f%%  Fan %s", readings.waterLevel, fanRunning ? "ON" : "OFF");
  display.setCursor(0, 54);
  display.printf("Pump %s   %s", waterPumpRunning ? "ON " : "OFF", readings.waterLevel < LOW_WATER_LEVEL ? "LOW WATER" : "OK");
  display.display();
}

void postReadingsToServer() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(API_URL);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(4000);

  JsonDocument payload;
  payload["type"] = "reading";
  payload["device_id"] = DEVICE_ID;
  payload["temperature"] = readings.temperature;
  payload["humidity"] = readings.humidity;
  payload["pressure"] = readings.pressure;
  payload["gas_resistance"] = readings.gasResistance;
  payload["ph"] = readings.ph;
  payload["tds"] = readings.tds;
  payload["water_level"] = readings.waterLevel;
  payload["fan_on"] = fanRunning;
  payload["water_pump_on"] = waterPumpRunning;

  String body;
  serializeJson(payload, body);
  const int httpCode = http.POST(body);
  Serial.printf("POST /api.php -> %d\n", httpCode);
  http.end();
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RELAY_PH_UP, OUTPUT);
  pinMode(RELAY_WATER, OUTPUT);
  pinMode(RELAY_FAN, OUTPUT);
  pinMode(RELAY_LIGHTS, OUTPUT);
  setRelay(RELAY_PH_UP, false);
  setRelay(RELAY_WATER, false);
  setRelay(RELAY_FAN, false);
  setRelay(RELAY_LIGHTS, false);

  Wire.begin();
  oledReady = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  if (oledReady) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("HYDRONEXUS BOOTING...");
    display.display();
  }

  bmeReady = bme.begin(0x76);
  if (!bmeReady) bmeReady = bme.begin(0x77);
  if (bmeReady) {
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  } else {
    Serial.println("BME680 not found; temperature/humidity will keep their defaults.");
  }

  connectToWifi();
}

void loop() {
  readSensors();
  regulateRelays();
  drawOLED();

  if (millis() - lastPostAt >= POST_INTERVAL_MS) {
    lastPostAt = millis();
    postReadingsToServer();
  }

  delay(500);
}
