#pragma once

#include <Arduino.h>
#include <time.h>
#include "sensors.h"

struct ActuatorState {
  bool phDosing = false;
  bool nutrientDosing = false;
  bool waterPump = false;
  bool exhaustFan = false;
  bool growLights = false;
  bool dryRunLockout = false;
};

void actuatorsBegin();
void actuatorsUpdate(const SensorReadings& readings, const struct tm& localTime);
void setManualRelay(const String& relayName, bool enabled);
const ActuatorState& actuatorState();
