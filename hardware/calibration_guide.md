# Sensor Calibration Guide

Calibration is part of the system, not an optional software setting. Record the date, solution brand, temperature, raw ADC voltage, and final calibration values in your lab notebook.

## pH probe calibration

You need fresh pH 7.00 and pH 4.01 buffer solutions, clean rinse water, lint-free tissue, and a stable container.

1. Power the pH interface board for at least 10 minutes so its output is stable.
2. Rinse the probe with distilled water and gently blot it. Do not wipe the glass bulb.
3. Place it in pH 7.00 buffer, gently stir, wait for a stable reading, and record the sensor output voltage.
4. Place it in pH 4.01 buffer, repeat the rinse and wait process, and record the voltage.
5. Calculate the slope using `slope = (V7 - V4) / (7.00 - 4.01)` and update `PH_VOLTAGE_PER_PH` and `PH_NEUTRAL_VOLTAGE` in `firmware/config.h`.
6. Rinse again and validate against both buffers. The display should be within the probe/interface accuracy specification.
7. Recheck calibration weekly during active dosing and any time the probe has dried out.

Keep the probe wet in the manufacturer's storage solution. Never store it dry or in distilled water for long periods.

## TDS / EC calibration

TDS probes are sensitive to temperature, cable placement, and the specific conversion factor used by the sensor board.

1. Prepare a known conductivity or TDS reference solution, commonly 342 ppm NaCl equivalent.
2. Rinse the probe with a small amount of the same reference solution.
3. Place the probe in the solution without touching the container. Wait for the reading to settle.
4. Record the water temperature and the raw sensor voltage.
5. Adjust `TDS_K_VALUE` in `firmware/config.h` until the displayed ppm matches the reference at 25°C.
6. Repeat at a second concentration if possible, then validate in clean nutrient solution.
7. Rinse after every use and do not leave the probe in concentrated nutrient solution when the system is powered down.

The firmware applies a starter temperature compensation around 25°C. This is not a substitute for the compensation curve supplied with your specific TDS board.

## Ultrasonic water-level calibration

1. Measure the sensor-to-water distance when the reservoir is empty and update `TANK_EMPTY_DISTANCE_CM`.
2. Fill to the maximum safe level, measure again, and update `TANK_FULL_DISTANCE_CM`.
3. Confirm that the reported percentage increases as water rises.
4. Test the dry-run lockout by lowering the water below `MINIMUM_SAFE_WATER_LEVEL_PERCENT` while the circulation pump is disconnected.

## Safe commissioning order

Run sensors first with every relay load disconnected. Then test each relay with a multimeter or indicator lamp. Finally connect one low-voltage pump at a time and verify that the ESP32 does not reset during startup current.
