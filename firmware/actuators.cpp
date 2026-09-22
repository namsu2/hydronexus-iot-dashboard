#include "actuators.h"
#include "config.h"

namespace {
ActuatorState state;
unsigned long phDoseStartedAt = 0;
unsigned long nutrientDoseStartedAt = 0;
unsigned long lastDoseEndedAt = 0;
bool manualOverride[4] = {false, false, false, false};

void relayWrite(uint8_t pin, bool enabled) {
  digitalWrite(pin, enabled ? RELAY_ON_LEVEL : RELAY_OFF_LEVEL);
}

int relayIndex(const String& name) {
  if (name == "ph_up") return 0;
  if (name == "water_pump") return 1;
  if (name == "fan") return 2;
  if (name == "lights") return 3;
  return -1;
}
}

void actuatorsBegin() {
  pinMode(RELAY_PH_UP_PIN, OUTPUT);
  pinMode(RELAY_WATER_PUMP_PIN, OUTPUT);
  pinMode(RELAY_FAN_PIN, OUTPUT);
  pinMode(RELAY_GROW_LIGHTS_PIN, OUTPUT);
  relayWrite(RELAY_PH_UP_PIN, false);
  relayWrite(RELAY_WATER_PUMP_PIN, false);
  relayWrite(RELAY_FAN_PIN, false);
  relayWrite(RELAY_GROW_LIGHTS_PIN, false);
}

void setManualRelay(const String& relayName, bool enabled) {
  const int index = relayIndex(relayName);
  if (index < 0) return;
  manualOverride[index] = enabled;
  if (index == 0) state.phDosing = enabled;
  if (index == 1) state.waterPump = enabled;
  if (index == 2) state.exhaustFan = enabled;
  if (index == 3) state.growLights = enabled;
}

void actuatorsUpdate(const SensorReadings& readings, const struct tm& localTime) {
  const unsigned long now = millis();

  // Hard safety edge: never let the circulation pump dry-run.
  state.dryRunLockout = readings.waterLevelPercent < MINIMUM_SAFE_WATER_LEVEL_PERCENT || !readings.waterLevelValid;
  if (state.dryRunLockout) state.waterPump = false;
  else if (!manualOverride[1]) {
    if (!state.waterPump && readings.waterLevelPercent < WATER_PUMP_START_LEVEL_PERCENT) state.waterPump = true;
    if (state.waterPump && readings.waterLevelPercent > WATER_PUMP_STOP_LEVEL_PERCENT) state.waterPump = false;
  }

  // Temperature hysteresis prevents rapid fan switching around 35°C.
  if (!manualOverride[2]) {
    if (!state.exhaustFan && readings.temperatureC > TEMPERATURE_FAN_ON_C) state.exhaustFan = true;
    if (state.exhaustFan && readings.temperatureC < TEMPERATURE_FAN_OFF_C) state.exhaustFan = false;
  }

  // A simple timed pH correction pulse, followed by a lockout for mixing.
  const bool dosingAllowed = now - lastDoseEndedAt > DOSING_LOCKOUT_MS;
  if (!manualOverride[0] && dosingAllowed && readings.ph < PH_MIN && phDoseStartedAt == 0) phDoseStartedAt = now;
  if (phDoseStartedAt > 0 && now - phDoseStartedAt >= PH_DOSING_PULSE_MS) {
    phDoseStartedAt = 0;
    lastDoseEndedAt = now;
  }
  state.phDosing = manualOverride[0] || phDoseStartedAt > 0;

  // LED schedule: use NTP time when available, but allow a manual override.
  if (!manualOverride[3]) {
    const unsigned long hour = static_cast<unsigned long>(localTime.tm_hour);
    state.growLights = hour >= LIGHTS_ON_HOUR && hour < LIGHTS_OFF_HOUR;
  }

  relayWrite(RELAY_PH_UP_PIN, state.phDosing && !state.dryRunLockout);
  relayWrite(RELAY_WATER_PUMP_PIN, state.waterPump && !state.dryRunLockout);
  relayWrite(RELAY_FAN_PIN, state.exhaustFan);
  relayWrite(RELAY_GROW_LIGHTS_PIN, state.growLights);
}

const ActuatorState& actuatorState() {
  return state;
}
