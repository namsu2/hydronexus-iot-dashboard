<?php
/**
 * HydroNexus API
 *
 * POST {"type":"reading", ...} from the ESP32 stores a sensor snapshot.
 * POST {"type":"relay", "relay":"fan", "state":true} updates a relay.
 * GET ?action=latest returns the latest reading and relay states for the UI.
 */

declare(strict_types=1);
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Headers: Content-Type');
header('Access-Control-Allow-Methods: GET, POST, OPTIONS');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(204);
    exit;
}

// Set these values in your hosting environment instead of committing secrets.
$dbHost = getenv('HYDRO_DB_HOST') ?: '127.0.0.1';
$dbName = getenv('HYDRO_DB_NAME') ?: 'hydronexus';
$dbUser = getenv('HYDRO_DB_USER') ?: 'hydronexus_user';
$dbPass = getenv('HYDRO_DB_PASS') ?: 'change-me';

try {
    $pdo = new PDO(
        "mysql:host={$dbHost};dbname={$dbName};charset=utf8mb4",
        $dbUser,
        $dbPass,
        [PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION, PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC]
    );
} catch (PDOException $error) {
    http_response_code(500);
    echo json_encode(['ok' => false, 'error' => 'Database connection failed.']);
    exit;
}

function jsonInput(): array
{
    $body = file_get_contents('php://input');
    $data = json_decode($body ?: '{}', true);
    return is_array($data) ? $data : [];
}

function numberField(array $data, string $key): float
{
    if (!isset($data[$key]) || !is_numeric($data[$key])) {
        throw new InvalidArgumentException("Missing numeric field: {$key}");
    }
    return (float) $data[$key];
}

try {
    if ($_SERVER['REQUEST_METHOD'] === 'GET') {
        $latest = $pdo->query('SELECT * FROM sensor_readings ORDER BY id DESC LIMIT 1')->fetch();
        $relayRows = $pdo->query('SELECT relay_name, state, mode FROM relay_states ORDER BY relay_name')->fetchAll();
        $relays = [];
        foreach ($relayRows as $row) {
            $relays[$row['relay_name']] = (bool) $row['state'];
        }
        echo json_encode(['ok' => true, 'reading' => $latest ?: null, 'relays' => $relays]);
        exit;
    }

    $payload = jsonInput();
    $type = $payload['type'] ?? '';

    if ($type === 'reading') {
        $stmt = $pdo->prepare(
            'INSERT INTO sensor_readings
             (device_id, temperature, humidity, pressure, gas_resistance, ph, tds, water_level, fan_on, water_pump_on)
             VALUES (:device_id, :temperature, :humidity, :pressure, :gas_resistance, :ph, :tds, :water_level, :fan_on, :water_pump_on)'
        );
        $stmt->execute([
            ':device_id' => substr((string)($payload['device_id'] ?? 'unknown'), 0, 80),
            ':temperature' => numberField($payload, 'temperature'),
            ':humidity' => numberField($payload, 'humidity'),
            ':pressure' => numberField($payload, 'pressure'),
            ':gas_resistance' => numberField($payload, 'gas_resistance'),
            ':ph' => numberField($payload, 'ph'),
            ':tds' => numberField($payload, 'tds'),
            ':water_level' => numberField($payload, 'water_level'),
            ':fan_on' => !empty($payload['fan_on']) ? 1 : 0,
            ':water_pump_on' => !empty($payload['water_pump_on']) ? 1 : 0,
        ]);
        echo json_encode(['ok' => true, 'message' => 'Reading stored.']);
        exit;
    }

    if ($type === 'relay') {
        $allowedRelays = ['ph_up', 'water_pump', 'fan', 'lights'];
        $relay = (string)($payload['relay'] ?? '');
        if (!in_array($relay, $allowedRelays, true)) {
            throw new InvalidArgumentException('Unknown relay.');
        }
        $stmt = $pdo->prepare(
            'INSERT INTO relay_states (relay_name, state, mode)
             VALUES (:relay, :state, "manual")
             ON DUPLICATE KEY UPDATE state = VALUES(state), mode = "manual", updated_at = CURRENT_TIMESTAMP'
        );
        $stmt->execute([':relay' => $relay, ':state' => !empty($payload['state']) ? 1 : 0]);
        echo json_encode(['ok' => true, 'message' => 'Relay state updated.']);
        exit;
    }

    throw new InvalidArgumentException('Use type=reading or type=relay.');
} catch (InvalidArgumentException $error) {
    http_response_code(400);
    echo json_encode(['ok' => false, 'error' => $error->getMessage()]);
} catch (Throwable $error) {
    http_response_code(500);
    echo json_encode(['ok' => false, 'error' => 'Request could not be completed.']);
}
