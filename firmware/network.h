#pragma once

#include <Arduino.h>
#include <time.h>
#include "sensors.h"
#include "actuators.h"

void networkBegin();
void networkUpdate();
void networkSendTelemetry(const SensorReadings& readings, const ActuatorState& actuators);
bool networkIsConnected();
struct tm networkLocalTime();
