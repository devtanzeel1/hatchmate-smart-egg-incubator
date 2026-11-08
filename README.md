# HatchMate — ESP32 Smart Egg Incubator Prototype

HatchMate is a team-built smart egg incubator prototype using an ESP32, Wi-Fi connectivity, sensor monitoring, relay-controlled outputs, an LCD, and the Blynk IoT platform.

> **Project status:** Educational prototype. The firmware in this repository has been cleaned up from the supplied project source, but it has not been compiled against the exact installed library versions or tested on the physical incubator after these changes. Do not use it for unattended incubation.

## Overview

The ESP32 reads temperature and humidity from a DHT11 sensor and measures distance to a water surface using an ultrasonic sensor. Readings are shown on a 20×4 I²C LCD and sent to Blynk over Wi-Fi.

The firmware also drives relay outputs for the heating, humidity-related device, fan, water pump, and tray mechanism. Relay polarity and connected loads must be checked against the actual hardware before power is applied.

## Features represented in the supplied prototype source

- ESP32-based controller with Wi-Fi connectivity
- DHT11 temperature and humidity sensing
- Ultrasonic water-distance measurement
- 20×4 I²C LCD status display
- Relay outputs for heater, humidity-related device, fan, pump, and tray mechanism
- Blynk telemetry using virtual pins V0, V1, and V2
- Timed tray-relay test pulse (details below)

## Hardware identified in the source code

- ESP32 development board
- DHT11 temperature/humidity sensor
- 20×4 I²C LCD module (address configured as `0x27`)
- Ultrasonic distance sensor
- Relay module(s)
- Heating load, humidity-related load, fan, water pump, and tray motor/mechanism
- Appropriate power supplies, wiring, and protection for the connected loads

The exact sensor module models, relay board channel count, load ratings, and power-supply specifications should be confirmed from the physical build before this repository is treated as final wiring documentation.

## Pin mapping in the cleaned firmware

| Function | ESP32 GPIO |
|---|---:|
| DHT11 data | 15 |
| Heater relay | 33 |
| Humidity relay | 32 |
| Fan relay | 25 |
| Pump relay | 14 |
| Tray relay | 26 |
| Ultrasonic trigger | 12 |
| Ultrasonic echo | 13 |
| LCD SDA | 21* |
| LCD SCL | 22* |

`*` GPIO 21/22 are common default I²C pins for many ESP32 DevKit boards. Confirm the exact board and wiring. GPIO 12 and GPIO 15 are ESP32 strapping pins; check boot behaviour with the attached circuit.

## Blynk datastream mapping

| Virtual pin | Value |
|---|---|
| V0 | Temperature (°C) |
| V1 | Relative humidity (%) |
| V2 | Ultrasonic distance (cm) |

The original source used a Blynk template ID and a device authentication token. The token and Wi-Fi credentials are intentionally not included in this public repository.

## Software dependencies

Install the ESP32 board support package and these Arduino libraries:

- Blynk
- DHTesp
- LiquidCrystal_I2C

`Wire.h`, `Arduino.h`, and the ESP32 Wi-Fi support are provided by the Arduino/ESP32 platform.

## Setup

1. Open this repository folder in your local development environment.
2. Copy `secrets.example.h` to a new local file named `secrets.h`.
3. Put your **newly generated** Blynk device token and your own Wi-Fi credentials in `secrets.h`.
4. Confirm the Blynk template name/ID and datastreams in your Blynk project.
5. Open `hatchmate-smart-egg-incubator.ino` in Arduino IDE.
6. Select the correct ESP32 board and port.
7. Verify the pin map, relay polarity, sensor voltage levels, and power wiring before compiling or connecting loads.
8. Compile and test with the loads disconnected or safely isolated first.

Do not upload `secrets.h`. The `.gitignore` file excludes it.

## Important prototype timing limitation

The supplied source energises the tray relay for approximately 5 seconds during seconds 10–15 of every 15-second cycle. This short interval is preserved as a **bench-test/demo behaviour only**. It is not a suitable real egg-turning schedule and must be replaced with a properly researched and validated schedule before any real incubation use.

## Safety and validation

Read [`docs/SAFETY_AND_VALIDATION.md`](docs/SAFETY_AND_VALIDATION.md) and [`docs/HARDWARE_WIRING.md`](docs/HARDWARE_WIRING.md) before connecting hardware. The prototype is not a certified incubator or a safety-rated control system.

## Project contributions

This is a team project. This repository is maintained under the GitHub account [`devtanzeel1`](https://github.com/devtanzeel1). Individual contributions should be described accurately in the project report and CV.
