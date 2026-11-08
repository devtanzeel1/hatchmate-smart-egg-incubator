/*
  HatchMate — ESP32 Smart Egg Incubator Prototype

  Refactored from the supplied "BLYNK 2 oky.txt" source.
  This is a documentation/cleanup candidate, not a physically tested release.

  IMPORTANT:
  - Copy secrets.example.h to secrets.h and enter your NEW credentials locally.
  - secrets.h is excluded by .gitignore and must never be committed.
  - Relay outputs are assumed ACTIVE-LOW, matching the supplied source.
  - The 15-second tray relay cycle is preserved only as a bench-test pulse.
    It is NOT a real egg-turning schedule.
*/

#define BLYNK_PRINT Serial
#include "secrets.h"  // Local file; do not commit.

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHTesp.h>
#include <BlynkSimpleEsp32.h>
#include <math.h>

// ------------------------- Hardware configuration -------------------------
constexpr uint8_t DHT_PIN = 15;
constexpr uint8_t TEMP_RELAY_PIN = 33;
constexpr uint8_t HUMIDITY_RELAY_PIN = 32;
constexpr uint8_t FAN_RELAY_PIN = 25;
constexpr uint8_t PUMP_RELAY_PIN = 14;
constexpr uint8_t TRAY_RELAY_PIN = 26;
constexpr uint8_t ULTRASONIC_TRIG_PIN = 12;
constexpr uint8_t ULTRASONIC_ECHO_PIN = 13;

constexpr uint8_t LCD_I2C_ADDRESS = 0x27;
constexpr uint8_t LCD_COLUMNS = 20;
constexpr uint8_t LCD_ROWS = 4;

// Common ESP32 DevKit default I2C pins. Confirm against the actual wiring.
constexpr uint8_t I2C_SDA_PIN = 21;
constexpr uint8_t I2C_SCL_PIN = 22;

// The original source drives relay inputs LOW to activate them.
constexpr uint8_t RELAY_ON_LEVEL = LOW;
constexpr uint8_t RELAY_OFF_LEVEL = HIGH;

// ------------------------- Control thresholds -----------------------------
// These values are copied from the supplied prototype source. They are not
// validated incubation set-points and must be calibrated for the hardware.
constexpr float HEATER_LOW_THRESHOLD_C = 36.0f;
constexpr float HEATER_HIGH_THRESHOLD_C = 39.0f;
constexpr float HUMIDITY_ON_THRESHOLD_PERCENT = 50.0f;
constexpr float FAN_HUMIDITY_THRESHOLD_PERCENT = 56.0f;
constexpr float FAN_TEMPERATURE_THRESHOLD_C = 40.0f;
constexpr float PUMP_DISTANCE_THRESHOLD_CM = 20.0f;

// Preserved from the supplied code as a short demonstration/bench-test cycle:
// tray relay ON during seconds 10–15 of each 15-second cycle.
constexpr uint32_t TRAY_CYCLE_MS = 15000UL;
constexpr uint32_t TRAY_ON_START_MS = 10000UL;

// ------------------------- Scheduling -------------------------------------
constexpr uint32_t SENSOR_READ_INTERVAL_MS = 2500UL;
constexpr uint32_t LCD_UPDATE_INTERVAL_MS = 750UL;
constexpr uint32_t BLYNK_SEND_INTERVAL_MS = 10000UL;
constexpr uint32_t ULTRASONIC_TIMEOUT_US = 30000UL;

DHTesp dht;
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLUMNS, LCD_ROWS);
BlynkTimer blynkTimer;

float currentTemperatureC = NAN;
float currentHumidityPercent = NAN;
float currentWaterDistanceCm = NAN;

bool dhtReadingValid = false;
bool waterDistanceValid = false;

bool heaterIsOn = false;
bool humidityRelayIsOn = false;
bool fanIsOn = false;
bool pumpIsOn = false;
bool trayRelayIsOn = false;

uint32_t lastSensorReadMs = 0;
uint32_t lastLcdUpdateMs = 0;

// ------------------------- Utility functions ------------------------------
void writeRelay(uint8_t pin, bool turnOn) {
  digitalWrite(pin, turnOn ? RELAY_ON_LEVEL : RELAY_OFF_LEVEL);
}

void printLcdLine(uint8_t row, const char *text) {
  if (row >= LCD_ROWS) return;

  lcd.setCursor(0, row);
  for (uint8_t column = 0; column < LCD_COLUMNS; ++column) {
    const char character = (text != nullptr && text[column] != '\0')
                               ? text[column]
                               : ' ';
    lcd.print(character);
  }
}

bool readWaterDistance(float &distanceCm) {
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

  const unsigned long echoDurationUs =
      pulseIn(ULTRASONIC_ECHO_PIN, HIGH, ULTRASONIC_TIMEOUT_US);

  if (echoDurationUs == 0UL) {
    return false;  // No echo within timeout; do not treat it as 0 cm.
  }

  distanceCm = (static_cast<float>(echoDurationUs) * 0.0343f) / 2.0f;
  return isfinite(distanceCm);
}

void applyRelayOutputs() {
  writeRelay(TEMP_RELAY_PIN, heaterIsOn);
  writeRelay(HUMIDITY_RELAY_PIN, humidityRelayIsOn);
  writeRelay(FAN_RELAY_PIN, fanIsOn);
  writeRelay(PUMP_RELAY_PIN, pumpIsOn);
  writeRelay(TRAY_RELAY_PIN, trayRelayIsOn);
}

void updateControlsFromReadings() {
  if (dhtReadingValid) {
    // Preserve the supplied source's relay behaviour:
    // LOW/ON below 36 C and also in the 36–39 C band; OFF above 39 C.
    // The old LCD text said OFF in the middle band even though the relay
    // output was still active. This version displays the commanded state.
    if (currentTemperatureC < HEATER_LOW_THRESHOLD_C) {
      heaterIsOn = true;
    } else if (currentTemperatureC > HEATER_HIGH_THRESHOLD_C) {
      heaterIsOn = false;
    } else {
      heaterIsOn = true;  // Matches the LOW output in the supplied source.
    }

    // The supplied source turns this relay ON below 50%; otherwise OFF.
    humidityRelayIsOn =
        currentHumidityPercent < HUMIDITY_ON_THRESHOLD_PERCENT;

    fanIsOn = (currentHumidityPercent > FAN_HUMIDITY_THRESHOLD_PERCENT) ||
              (currentTemperatureC > FAN_TEMPERATURE_THRESHOLD_C);
  }

  // If the ultrasonic sensor has no echo, keep the previous pump command.
  if (waterDistanceValid) {
    pumpIsOn = currentWaterDistanceCm > PUMP_DISTANCE_THRESHOLD_CM;
  }

  // This intentionally preserves the short test pulse from the original code.
  const uint32_t trayCyclePositionMs = millis() % TRAY_CYCLE_MS;
  trayRelayIsOn = trayCyclePositionMs >= TRAY_ON_START_MS;

  applyRelayOutputs();
}

void readSensors() {
  const float temperature = dht.getTemperature();
  const float humidity = dht.getHumidity();

  dhtReadingValid = isfinite(temperature) && isfinite(humidity);
  if (dhtReadingValid) {
    currentTemperatureC = temperature;
    currentHumidityPercent = humidity;
  }

  float measuredDistanceCm = NAN;
  waterDistanceValid = readWaterDistance(measuredDistanceCm);
  if (waterDistanceValid) {
    currentWaterDistanceCm = measuredDistanceCm;
  }

  updateControlsFromReadings();
}

void sendSensorDataToBlynk() {
  if (dhtReadingValid) {
    Blynk.virtualWrite(V0, currentTemperatureC);
    Blynk.virtualWrite(V1, currentHumidityPercent);
  }

  if (waterDistanceValid) {
    Blynk.virtualWrite(V2, currentWaterDistanceCm);
  }
}

void updateLcd() {
  char line[LCD_COLUMNS + 1] = {0};

  if (dhtReadingValid) {
    snprintf(line, sizeof(line), "T:%.1fC H:%.1f%%",
             currentTemperatureC, currentHumidityPercent);
  } else {
    snprintf(line, sizeof(line), "HatchMate ESP32");
  }
  printLcdLine(0, line);

  if (waterDistanceValid) {
    snprintf(line, sizeof(line), "Water: %.1f cm", currentWaterDistanceCm);
  } else {
    snprintf(line, sizeof(line), "Water: no echo");
  }
  printLcdLine(1, line);

  snprintf(line, sizeof(line), "Heat:%s Hum:%s",
           heaterIsOn ? "ON" : "OFF",
           humidityRelayIsOn ? "ON" : "OFF");
  printLcdLine(2, line);

  snprintf(line, sizeof(line), "Fan:%s Pump:%s",
           fanIsOn ? "ON" : "OFF",
           pumpIsOn ? "ON" : "OFF");
  printLcdLine(3, line);
}

// ------------------------- Arduino setup/loop -----------------------------
void setup() {
  Serial.begin(115200);

  // Set relay outputs inactive before starting sensors/network services.
  const uint8_t relayPins[] = {
      TEMP_RELAY_PIN,
      HUMIDITY_RELAY_PIN,
      FAN_RELAY_PIN,
      PUMP_RELAY_PIN,
      TRAY_RELAY_PIN};

  for (uint8_t index = 0; index < sizeof(relayPins); ++index) {
    pinMode(relayPins[index], OUTPUT);
    digitalWrite(relayPins[index], RELAY_OFF_LEVEL);
  }

  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

  dht.setup(DHT_PIN, DHTesp::DHT11);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  lcd.init();
  lcd.backlight();
  printLcdLine(0, "HatchMate ESP32");
  printLcdLine(1, "Starting sensors...");
  printLcdLine(2, "Connecting Wi-Fi...");
  printLcdLine(3, "Prototype firmware");

  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);
  blynkTimer.setInterval(BLYNK_SEND_INTERVAL_MS, sendSensorDataToBlynk);

  // Force the first sensor read on the first loop pass.
  lastSensorReadMs = millis() - SENSOR_READ_INTERVAL_MS;
}

void loop() {
  Blynk.run();
  blynkTimer.run();

  const uint32_t now = millis();

  if (now - lastSensorReadMs >= SENSOR_READ_INTERVAL_MS) {
    lastSensorReadMs = now;
    readSensors();
  }

  if (now - lastLcdUpdateMs >= LCD_UPDATE_INTERVAL_MS) {
    lastLcdUpdateMs = now;
    updateLcd();
  }
}
