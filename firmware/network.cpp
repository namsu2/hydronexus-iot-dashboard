#include "network.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "config.h"

namespace {
unsigned long lastReconnectAttempt = 0;
bool timeConfigured = false;
}

void networkBegin() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  configTime(UTC_OFFSET_SECONDS, DST_OFFSET_SECONDS, NTP_SERVER);
}

void networkUpdate() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!timeConfigured) {
      struct tm timeInfo;
      timeConfigured = getLocalTime(&timeInfo, 1000);
    }
    return;
  }

  const unsigned long now = millis();
  if (now - lastReconnectAttempt < 10000UL) return;
  lastReconnectAttempt = now;
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

bool networkIsConnected() {
  return WiFi.status() == WL_CONNECTED;
}

struct tm networkLocalTime() {
  struct tm timeInfo{};
  if (!getLocalTime(&timeInfo, 50)) {
    timeInfo.tm_hour = 12; // Safe daytime default while NTP is unavailable.
  }
  return timeInfo;
}

void networkSendTelemetry(const SensorReadings& readings, const ActuatorState& actuators) {
  if (!networkIsConnected()) return;

  HTTPClient http;
  http.begin(API_URL);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(4000);

  JsonDocument payload;
  payload["type"] = "reading";
  payload["device_id"] = HYDRONEXUS_DEVICE_ID;
  payload["temperature"] = readings.temperatureC;
  payload["humidity"] = readings.humidityPercent;
  payload["pressure"] = readings.pressureHpa;
  payload["gas_resistance"] = readings.gasResistanceKOhm;
  payload["ph"] = readings.ph;
  payload["tds"] = readings.tdsPpm;
  payload["ec"] = readings.ecUsCm;
  payload["water_level"] = readings.waterLevelPercent;
  payload["fan_on"] = actuators.exhaustFan;
  payload["water_pump_on"] = actuators.waterPump;
  payload["dry_run_lockout"] = actuators.dryRunLockout;

  String body;
  serializeJson(payload, body);
  const int responseCode = http.POST(body);
  Serial.printf("Telemetry POST -> %d\n", responseCode);
  http.end();
}
