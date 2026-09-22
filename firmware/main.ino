/*
  HydroNexus modular ESP32 firmware

  Keep this file intentionally small: each subsystem owns its own hardware and
  can be tested independently. The local safety loop never depends on Wi-Fi.
*/
#include "config.h"
#include "sensors.h"
#include "actuators.h"
#include "oled_display.h"
#include "network.h"

SensorReadings readings;
unsigned long lastSensorSampleAt = 0;
unsigned long lastActuatorUpdateAt = 0;
unsigned long lastDisplayUpdateAt = 0;
unsigned long lastTelemetryPostAt = 0;

void setup() {
  Serial.begin(115200);
  delay(100);
  sensorsBegin();
  actuatorsBegin();
  oledDisplayBegin();
  networkBegin();
}

void loop() {
  const unsigned long now = millis();
  networkUpdate();
  const struct tm localTime = networkLocalTime();

  // Sensor sampling is periodic and non-blocking from the loop's perspective.
  if (now - lastSensorSampleAt >= SENSOR_SAMPLE_INTERVAL_MS) {
    lastSensorSampleAt = now;
    sensorsRead(readings);
  }

  // Edge autonomy: actuator decisions run locally even when Wi-Fi is offline.
  if (now - lastActuatorUpdateAt >= ACTUATOR_INTERVAL_MS) {
    lastActuatorUpdateAt = now;
    actuatorsUpdate(readings, localTime);
  }

  if (now - lastDisplayUpdateAt >= SENSOR_SAMPLE_INTERVAL_MS) {
    lastDisplayUpdateAt = now;
    oledDisplayUpdate(readings, actuatorState());
  }

  if (now - lastTelemetryPostAt >= CLOUD_POST_INTERVAL_MS) {
    lastTelemetryPostAt = now;
    networkSendTelemetry(readings, actuatorState());
  }
}
