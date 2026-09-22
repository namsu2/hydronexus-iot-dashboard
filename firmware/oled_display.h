#pragma once

#include "sensors.h"
#include "actuators.h"

void oledDisplayBegin();
void oledDisplayUpdate(const SensorReadings& readings, const ActuatorState& actuators);
