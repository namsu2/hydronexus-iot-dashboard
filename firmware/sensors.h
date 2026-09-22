#pragma once

#include <Arduino.h>
#include <Adafruit_BME680.h>
#include "config.h"

struct SensorReadings {
  float temperatureC = 0.0f;
  float humidityPercent = 0.0f;
  float pressureHpa = 0.0f;
  float gasResistanceKOhm = 0.0f;
  float ph = 7.0f;
  float tdsPpm = 0.0f;
  float ecUsCm = 0.0f;
  float waterLevelPercent = 0.0f;
  bool bme680Available = false;
  bool waterLevelValid = false;
};

void sensorsBegin();
void sensorsRead(SensorReadings& readings);
float readPhValue(float temperatureC);
float readTdsPpm(float temperatureC);
float readWaterLevelPercent();
const SensorReadings& latestSensorReadings();
