# HydroNexus Pinout Mapping

This wiring plan assumes an ESP32 DevKit V1, an active-LOW optocoupled 4-channel relay board, a 3.3 V-safe analog front end, and a common-ground low-voltage installation.

## Pin map

| Subsystem | Module pin | ESP32 pin | Supply | Wiring notes |
|---|---|---:|---|---|
| pH sensor | Analog out | GPIO 34 / ADC1 | 3.3 V signal | GPIO 34 is input-only. Never exceed 3.3 V. |
| TDS/EC sensor | Analog out | GPIO 35 / ADC1 | 3.3 V signal | GPIO 35 is input-only. Use the sensor board's conditioned output. |
| BME680 | SDA | GPIO 21 | 3.3 V | I²C address usually `0x76` or `0x77`. |
| BME680 | SCL | GPIO 22 | 3.3 V | Add 4.7 kΩ pull-ups if the breakout does not include them. |
| OLED SSD1306 | SDA | GPIO 21 | 3.3 V | Typical address `0x3C`; share the I²C bus. |
| OLED SSD1306 | SCL | GPIO 22 | 3.3 V | Do not power a 5 V-only display from 3.3 V. |
| HC-SR04 | TRIG | GPIO 5 | 5 V module | ESP32 drives this 3.3 V logic high successfully on common modules. |
| HC-SR04 | ECHO | GPIO 18 | 5 V output | **Mandatory voltage divider or level shifter** before ESP32. |
| Relay 1 | IN1 | GPIO 16 | 5 V board supply | pH-up dosing pump. Active LOW by default. |
| Relay 2 | IN2 | GPIO 17 | 5 V board supply | Water circulation/refill pump. |
| Relay 3 | IN3 | GPIO 19 | 5 V board supply | Exhaust fan. |
| Relay 4 | IN4 | GPIO 23 | 5 V board supply | LED grow lights. |
| All logic modules | GND | Common GND | — | Tie logic grounds together; keep load return currents separate. |

## Power architecture

Use a regulated **5 V adapter** for the ESP32 VIN/USB path, relay logic, HC-SR04, and compatible sensor breakout boards. Use a separate **12 V supply** sized for the pumps and fans. The relay contacts isolate the load voltage from the ESP32 logic side; the pump/fan supply must never be routed through the ESP32 regulator.

A typical relay load path is:

```text
12 V positive ── fuse ── relay COM
relay NO ─────────────── pump/fan positive
pump/fan negative ────── 12 V negative
```

Add a flyback diode or use a protected DC motor driver where the load is not already protected. The relay board should be optocoupled, have a transistor driver per channel, and be rated above the pump's startup current.

## HC-SR04 echo divider

A common divider that drops a 5 V echo signal to approximately 3.3 V is:

```text
HC-SR04 ECHO ── 1 kΩ ──┬── ESP32 GPIO 18
                       |
                      2 kΩ
                       |
                      GND
```

Check the actual resistor values and output with a multimeter before connecting the ESP32. A bidirectional level shifter is an alternative, but a simple divider is sufficient for the one-way ECHO signal.

## Grounding and noise

Keep the analog sensor wires away from pump and relay load wires. Use short shielded or twisted signal runs where practical. Put a 100 µF electrolytic and a 0.1 µF ceramic capacitor close to the relay board supply, and use a separate decoupled supply path for the ESP32 if pump switching resets the controller.
