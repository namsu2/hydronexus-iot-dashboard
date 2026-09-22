#include "oled_display.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

namespace {
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
bool available = false;
}

void oledDisplayBegin() {
  available = display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS);
  if (!available) return;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("HYDRONEXUS BOOTING");
  display.display();
}

void oledDisplayUpdate(const SensorReadings& readings, const ActuatorState& actuators) {
  if (!available) return;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("HYDRONEXUS | EDGE");
  display.drawLine(0, 11, 127, 11, SSD1306_WHITE);
  display.setCursor(0, 16);
  display.printf("T %4.1fC H %3.0f%%", readings.temperatureC, readings.humidityPercent);
  display.setCursor(0, 28);
  display.printf("pH %4.2f  TDS %4.0f", readings.ph, readings.tdsPpm);
  display.setCursor(0, 40);
  display.printf("Water %3.0f%% Fan %s", readings.waterLevelPercent, actuators.exhaustFan ? "ON" : "OFF");
  display.setCursor(0, 53);
  if (actuators.dryRunLockout) display.print("SAFETY: LOW WATER");
  else display.printf("Pump %s Light %s", actuators.waterPump ? "ON" : "OFF", actuators.growLights ? "ON" : "OFF");
  display.display();
}
