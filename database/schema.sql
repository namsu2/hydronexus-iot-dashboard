-- HydroNexus database schema
-- MySQL 8+ / MariaDB 10.5+

CREATE DATABASE IF NOT EXISTS hydronexus
  CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE hydronexus;

CREATE TABLE IF NOT EXISTS sensor_readings (
  id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  device_id VARCHAR(80) NOT NULL,
  temperature DECIMAL(5,2) NOT NULL,
  humidity DECIMAL(5,2) NOT NULL,
  pressure DECIMAL(7,2) NOT NULL DEFAULT 0,
  gas_resistance DECIMAL(9,2) NOT NULL DEFAULT 0,
  ph DECIMAL(4,2) NOT NULL,
  tds DECIMAL(8,2) NOT NULL,
  water_level DECIMAL(5,2) NOT NULL,
  fan_on TINYINT(1) NOT NULL DEFAULT 0,
  water_pump_on TINYINT(1) NOT NULL DEFAULT 0,
  created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (id),
  INDEX idx_sensor_readings_created_at (created_at),
  INDEX idx_sensor_readings_device_id (device_id)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS relay_states (
  relay_name ENUM('ph_up', 'water_pump', 'fan', 'lights') NOT NULL,
  state TINYINT(1) NOT NULL DEFAULT 0,
  mode ENUM('auto', 'manual') NOT NULL DEFAULT 'auto',
  updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (relay_name)
) ENGINE=InnoDB;

INSERT INTO relay_states (relay_name, state, mode) VALUES
  ('ph_up', 0, 'auto'),
  ('water_pump', 0, 'auto'),
  ('fan', 0, 'auto'),
  ('lights', 0, 'manual')
ON DUPLICATE KEY UPDATE relay_name = relay_name;

-- Optional least-privilege account for the PHP API.
-- Run these statements as a privileged MySQL administrator and replace the password.
-- CREATE USER 'hydronexus_user'@'localhost' IDENTIFIED BY 'replace-with-a-long-password';
-- GRANT SELECT, INSERT, UPDATE ON hydronexus.* TO 'hydronexus_user'@'localhost';
-- FLUSH PRIVILEGES;
