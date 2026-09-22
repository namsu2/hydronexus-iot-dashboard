#include "sensors.h"
#include <Wire.h>

namespace {
Adafruit_BME680 bme680;
SensorReadings latest;
unsigned long lastWaterReadAt = 0;
float lastWaterLevel = 0.0f;

float adcVoltage(uint8_t pin) {
  // Average a few samples to reduce ADC noise on the ESP32.
  uint32_t total = 0;
  constexpr uint8_t samples = 8;
  for (uint8_t i = 0; i < samples; ++i) {
    total += analogRead(pin);
    delayMicroseconds(250);
  }
  return (static_cast<float>(total) / samples) * ADC_REFERENCE_VOLTAGE / ADC_MAX_VALUE;
}
}

void sensorsBegin() {
  analogReadResolution(12);
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  latest.bme680Available = bme680.begin(0x76);
  if (!latest.bme680Available) latest.bme680Available = bme680.begin(0x77);
  if (latest.bme680Available) {
    bme680.setTemperatureOversampling(BME680_OS_8X);
    bme680.setHumidityOversampling(BME680_OS_2X);
    bme680.setPressureOversampling(BME680_OS_4X);
    bme680.setIIRFilterSize(BME680_FILTER_SIZE_3);
  }
}

float readPhValue(float temperatureC) {
  const float voltage = adcVoltage(PH_SENSOR_PIN);
  const float neutralOffset = PH_NEUTRAL_VOLTAGE - (temperatureC - TDS_TEMPERATURE_REFERENCE_C) * PH_TEMPERATURE_COEFFICIENT;
  const float ph = 7.0f + ((neutralOffset - voltage) / PH_VOLTAGE_PER_PH);
  return constrain(ph, 0.0f, 14.0f);
}

float readTdsPpm(float temperatureC) {
  const float voltage = adcVoltage(TDS_SENSOR_PIN);
  const float compensation = 1.0f + 0.02f * (temperatureC - TDS_TEMPERATURE_REFERENCE_C);
  const float compensatedVoltage = voltage / max(0.1f, compensation);
  const float ec = (133.42f * pow(compensatedVoltage, 3)
                 - 255.86f * pow(compensatedVoltage, 2)
                 + 857.39f * compensatedVoltage) * TDS_K_VALUE;
  return max(0.0f, ec);
}

float readWaterLevelPercent() {
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

  const unsigned long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, ULTRASONIC_TIMEOUT_US);
  if (duration == 0) {
    latest.waterLevelValid = false;
    return lastWaterLevel;
  }

  const float distance = duration * 0.0343f / 2.0f;
  const float percent = 100.0f * (TANK_EMPTY_DISTANCE_CM - distance)
                      / (TANK_EMPTY_DISTANCE_CM - TANK_FULL_DISTANCE_CM);
  latest.waterLevelValid = true;
  lastWaterLevel = constrain(percent, 0.0f, 100.0f);
  return lastWaterLevel;
}

void sensorsRead(SensorReadings& readings) {
  if (latest.bme680Available && bme680.performReading()) {
    readings.temperatureC = bme680.temperature;
    readings.humidityPercent = bme680.humidity;
    readings.pressureHpa = bme680.pressure / 100.0f;
    readings.gasResistanceKOhm = bme680.gas_resistance / 1000.0f;
  }

  readings.ph = readPhValue(readings.temperatureC);
  readings.tdsPpm = readTdsPpm(readings.temperatureC);
  readings.ecUsCm = readings.tdsPpm / 0.5f;
  readings.waterLevelPercent = readWaterLevelPercent();
  readings.bme680Available = latest.bme680Available;
  readings.waterLevelValid = latest.waterLevelValid;
  latest = readings;
}

const SensorReadings& latestSensorReadings() {
  return latest;
}
