# HydroNexus Automated IoT NFT Hydroponic System

HydroNexus is a beginner-friendly starter repository for an automated Nutrient Film Technique (NFT) lettuce grow system. It combines an ESP32 controller, environmental and nutrient sensors, a four-channel relay module, a PHP/MySQL telemetry API, a browser dashboard, and a Python growth-stage classifier.

> **Safety first:** Treat this repository as an educational starting point. Use a low-voltage DC supply where possible, keep mains wiring enclosed, add flyback/protection hardware for inductive loads, and have a qualified adult review any mains-powered pump, fan, or grow-light wiring.

## Repository layout

```text
firmware/main.ino       ESP32 sensor, OLED, relay, Wi-Fi, and HTTP code
dashboard/index.html    Standalone live dashboard
dashboard/style.css     Dashboard styling
dashboard/api.php       JSON API for readings and relay state
database/schema.sql     MySQL/MariaDB tables and starter relay rows
ml/predict.py           Lettuce growth-stage inference script
README.md               This setup and calibration guide
```

## Hardware wiring

| Module | ESP32 connection | Notes |
|---|---:|---|
| Analog pH sensor | GPIO 34 | ADC input only; use a conditioned 0–3.3 V output |
| Analog TDS sensor | GPIO 35 | ADC input only; keep the probe powered as recommended by its manufacturer |
| BME680 | I²C GPIO 21 SDA / GPIO 22 SCL | Common addresses are `0x76` and `0x77`; the sketch tries both |
| 0.96-inch OLED | I²C GPIO 21 SDA / GPIO 22 SCL | Typical SSD1306 address is `0x3C` |
| HC-SR04 TRIG | GPIO 5 | Use a voltage divider on ECHO for a 5 V HC-SR04 module |
| HC-SR04 ECHO | GPIO 18 | Protect the ESP32 input from 5 V |
| Relay 1 / pH dosing | GPIO 16 | pH-up dosing pump |
| Relay 2 / water pump | GPIO 17 | Reservoir circulation/refill pump |
| Relay 3 / fan | GPIO 19 | Climate fan |
| Relay 4 / grow lights | GPIO 23 | Grow-light power relay |
| All modules | Common GND | Use a suitable external supply for pumps and relays |

The firmware assumes the relay board is **active LOW**. If your module turns on with a HIGH signal, swap `RELAY_ON` and `RELAY_OFF` in `firmware/main.ino`.

## 1. Flash the ESP32 firmware

1. Install the Arduino IDE and add the ESP32 board package through **Boards Manager**.
2. Install these libraries through **Library Manager**: `Adafruit BME680`, `Adafruit SSD1306`, `Adafruit GFX Library`, and `ArduinoJson`.
3. Open `firmware/main.ino`.
4. Replace `WIFI_SSID`, `WIFI_PASSWORD`, and `API_URL` with your network and hosted API URL.
5. Select your ESP32 board and serial port, then upload.
6. Open Serial Monitor at `115200` baud and confirm the Wi-Fi address and API response code.

### Calibration checklist

The pH and TDS conversions in this educational sketch are initial approximations. Calibrate before using automatic dosing:

1. Put the pH probe in a known pH 7.00 buffer and update the voltage-to-pH relationship in `readPH()`.
2. Repeat with pH 4.00 to calculate the slope for your specific probe and amplifier.
3. Use a known TDS solution to tune the `readTDS()` polynomial and account for temperature compensation.
4. Measure the empty and full tank distances, then update `tankEmptyDistance` and `tankFullDistance`.
5. Test each relay with pumps disconnected before connecting plumbing or loads.

The firmware includes hysteresis for the fan and water pump so that relays do not chatter around a threshold. pH dosing is intentionally conservative and should be upgraded to a timed dosing state machine for a production system.

## 2. Set up the PHP API and database

You need PHP 8+ with PDO MySQL and MySQL 8+ or MariaDB.

```bash
# From a MySQL administrator account
mysql -u root -p < database/schema.sql

# Start a local PHP server from the dashboard directory
cd dashboard
php -S 0.0.0.0:8080
```

The API reads these environment variables. Set them in your host or shell instead of committing credentials:

```bash
export HYDRO_DB_HOST=127.0.0.1
export HYDRO_DB_NAME=hydronexus
export HYDRO_DB_USER=hydronexus_user
export HYDRO_DB_PASS='replace-with-a-long-password'
```

Open `http://localhost:8080/index.html`. The dashboard polls `api.php?action=latest` every five seconds. The ESP32 posts a JSON object with `type: "reading"` to the same endpoint.

### Test the API manually

```bash
curl -X POST http://localhost:8080/api.php \\
  -H 'Content-Type: application/json' \\
  -d '{
    "type":"reading",
    "device_id":"hydronexus-esp32-01",
    "temperature":24.8,
    "humidity":61,
    "pressure":1012.4,
    "gas_resistance":12.2,
    "ph":6.2,
    "tds":842,
    "water_level":78,
    "fan_on":false,
    "water_pump_on":true
  }'

curl 'http://localhost:8080/api.php?action=latest'
```

## 3. Run the lettuce growth classifier

The script accepts a single image. It can use a fine-tuned TorchScript/PyTorch model, or it can run immediately with a small OpenCV heuristic so you can verify the pipeline before training a model.

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install opencv-python numpy torch torchvision

# Starter mode: works without a checkpoint
python ml/predict.py --image ./samples/lettuce.jpg --json

# Model mode: put a TorchScript checkpoint in models/ or pass a custom path
python ml/predict.py \\
  --image ./samples/lettuce.jpg \\
  --model ./models/lettuce_stage.pt \\
  --json
```

The model should return three scores in this order: `seedling`, `vegetative`, `mature`. For a real classifier, collect images from your own grow room, label them consistently, split into train/validation sets, and export the trained model to a TorchScript-compatible file.

## Dashboard behavior

The standalone dashboard shows live gauges for temperature, humidity, pH, TDS, and water level. It also exposes four manual switches. Manual state changes are stored in `relay_states` with `mode = manual`; the current Arduino sketch still performs its local safety automation on the board. For a complete remote-control workflow, add an authenticated command queue that the ESP32 polls and acknowledge commands after a relay change.

## Suggested next improvements

- Add API authentication or a device token before exposing the endpoint publicly.
- Store pH/TDS calibration values in a configuration file or a protected settings table.
- Add alert notifications when water level or pH leaves the safe band.
- Add a dosing lockout, maximum daily dose, and a mixing delay after each dose.
- Train and validate a real lettuce stage classifier using images captured under consistent lighting.
- Add a real-time chart backed by recent rows from `sensor_readings`.

## License

Use, adapt, and learn from this starter project. Add a license appropriate for your class, lab, or GitHub repository before publishing.
