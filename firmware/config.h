#pragma once

#include <Arduino.h>

// ----------------------------- Device identity ----------------------------
#define HYDRONEXUS_DEVICE_ID "hydronexus-esp32-01"

// ----------------------------- Wi-Fi / NTP -------------------------------
// Keep real credentials out of GitHub. Replace these locally before flashing.
#define WIFI_SSID "YOUR_WIFI_NAME"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define API_URL "https://your-domain.example/dashboard/api.php"
#define NTP_SERVER "pool.ntp.org"
#define UTC_OFFSET_SECONDS 0
#define DST_OFFSET_SECONDS 0

// ------------------------------ I2C pins ---------------------------------
constexpr uint8_t I2C_SDA_PIN = 21;
constexpr uint8_t I2C_SCL_PIN = 22;
constexpr uint8_t OLED_I2C_ADDRESS = 0x3C;

// --------------------------- Analog sensors -------------------------------
constexpr uint8_t PH_SENSOR_PIN = 34;
constexpr uint8_t TDS_SENSOR_PIN = 35;
constexpr float ADC_REFERENCE_VOLTAGE = 3.3f;
constexpr float ADC_MAX_VALUE = 4095.0f;

// Calibrate these values with the procedures in hardware/calibration_guide.md.
constexpr float PH_NEUTRAL_VOLTAGE = 2.50f;
constexpr float PH_VOLTAGE_PER_PH = 0.18f;
constexpr float PH_TEMPERATURE_COEFFICIENT = 0.003f;
constexpr float TDS_K_VALUE = 0.5f;
constexpr float TDS_TEMPERATURE_REFERENCE_C = 25.0f;

// --------------------------- Ultrasonic sensor ----------------------------
constexpr uint8_t ULTRASONIC_TRIG_PIN = 5;
constexpr uint8_t ULTRASONIC_ECHO_PIN = 18;
constexpr float TANK_EMPTY_DISTANCE_CM = 35.0f;
constexpr float TANK_FULL_DISTANCE_CM = 5.0f;
constexpr unsigned long ULTRASONIC_TIMEOUT_US = 30000UL;

// ------------------------------- Relays ----------------------------------
constexpr uint8_t RELAY_PH_UP_PIN = 16;
constexpr uint8_t RELAY_WATER_PUMP_PIN = 17;
constexpr uint8_t RELAY_FAN_PIN = 19;
constexpr uint8_t RELAY_GROW_LIGHTS_PIN = 23;

// Most 4-channel boards are active LOW. Change these two values if needed.
constexpr uint8_t RELAY_ON_LEVEL = LOW;
constexpr uint8_t RELAY_OFF_LEVEL = HIGH;

// --------------------------- Safety thresholds ---------------------------
constexpr float PH_MIN = 5.5f;
constexpr float PH_MAX = 6.5f;
constexpr float TEMPERATURE_FAN_ON_C = 35.0f;
constexpr float TEMPERATURE_FAN_OFF_C = 33.0f;
constexpr float MINIMUM_SAFE_WATER_LEVEL_PERCENT = 35.0f;
constexpr float WATER_PUMP_START_LEVEL_PERCENT = 42.0f;
constexpr float WATER_PUMP_STOP_LEVEL_PERCENT = 86.0f;
constexpr float TDS_MIN_PPM = 600.0f;
constexpr float TDS_MAX_PPM = 1000.0f;

// --------------------------- Timing settings -----------------------------
constexpr unsigned long SENSOR_SAMPLE_INTERVAL_MS = 1000UL;
constexpr unsigned long ACTUATOR_INTERVAL_MS = 100UL;
constexpr unsigned long CLOUD_POST_INTERVAL_MS = 5000UL;
constexpr unsigned long PH_DOSING_PULSE_MS = 8000UL;
constexpr unsigned long NUTRIENT_DOSING_PULSE_MS = 5000UL;
constexpr unsigned long DOSING_LOCKOUT_MS = 120000UL;
constexpr unsigned long LIGHTS_ON_HOUR = 6UL;
constexpr unsigned long LIGHTS_OFF_HOUR = 22UL;

// --------------------------- Display settings ----------------------------
constexpr uint8_t OLED_WIDTH = 128;
constexpr uint8_t OLED_HEIGHT = 64;
