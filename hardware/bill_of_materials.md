# Bill of Materials

The quantities below describe one small NFT lettuce channel. Select final ratings from the datasheets of the exact modules you purchase.

| Component | Qty | Typical voltage | Planning notes |
|---|---:|---:|---|
| ESP32 DevKit V1 | 1 | 5 V USB / 3.3 V logic | Wi-Fi controller; ADC inputs are not 5 V tolerant. |
| Analog pH probe + interface board | 1 | 5 V board / 0–3.3 V analog | Use pH 4.01 and 7.00 buffers for calibration. |
| Analog TDS/EC probe + interface board | 1 | 3.3–5 V | Use a known conductivity/TDS reference solution. |
| BME680 breakout | 1 | 3.3 V | Temperature, humidity, pressure, gas resistance over I²C. |
| HC-SR04 ultrasonic sensor | 1 | 5 V | Use a divider/level shifter on ECHO. |
| 0.96-inch SSD1306 OLED | 1 | 3.3 V | I²C, commonly `0x3C`. |
| 4-channel optocoupled relay module | 1 | 5 V logic/coil | Confirm active-LOW input behavior and contact rating. |
| 12 V submersible circulation pump | 1 | 12 V | Size for the NFT flow rate and head height. |
| 12 V peristaltic dosing pumps | 2–3 | 12 V | Separate pH-up, pH-down, and nutrient channels as needed. |
| 12 V exhaust fan | 1 | 12 V | Select a sealed or protected unit for humid areas. |
| LED grow-light bar | 1 | Per light datasheet | Relay must be rated above inrush current. |
| 5 V regulated adapter | 1 | 5 V, ≥2 A recommended | Powers ESP32, relay logic, and 5 V modules. |
| 12 V regulated adapter | 1 | 12 V, current sized to loads | Add headroom for simultaneous pump startup. |
| Inline fuses | As needed | Load-specific | One fuse per external load branch is recommended. |
| Logic-level divider / shifter | 1+ | 5 V to 3.3 V | Required for HC-SR04 ECHO. |
| Waterproof project enclosure | 1 | — | Keep mains/load terminals isolated from water. |
| Silicone tubing, check valves, fittings | As needed | — | Use chemical-compatible tubing for dosing. |
| pH 4.01 / 7.00 buffers | 1 each | — | Replace when contaminated or expired. |
| TDS/EC reference solution | 1 | — | Choose a known ppm or conductivity value. |

## Sizing notes

- The 5 V adapter should cover the ESP32, relay board, OLED, BME680, HC-SR04, and analog interfaces without running at its limit.
- The 12 V adapter should cover the sum of pump and fan running currents **plus startup/inrush headroom**. If two pumps start together, size for that combined peak.
- For grow lights or any mains-powered load, use a properly enclosed, certified relay/contactor and have a qualified person inspect the installation.
