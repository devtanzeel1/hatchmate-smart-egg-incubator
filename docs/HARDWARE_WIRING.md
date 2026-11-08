# Hardware Wiring Reference

This table records the GPIO assignments found in the selected ESP32+Blynk source file and the I²C defaults added during cleanup. It is a **software pin reference**, not a verified circuit diagram.

| Device / function | ESP32 GPIO / setting | Source of information |
|---|---:|---|
| DHT11 data | GPIO 15 | Selected source file |
| Heater relay | GPIO 33 | Selected source file |
| Humidity-related relay | GPIO 32 | Selected source file |
| Fan relay | GPIO 25 | Selected source file |
| Water-pump relay | GPIO 14 | Selected source file |
| Tray-mechanism relay | GPIO 26 | Selected source file |
| Ultrasonic TRIG | GPIO 12 | Selected source file |
| Ultrasonic ECHO | GPIO 13 | Selected source file |
| LCD I²C address | `0x27` | Selected source file |
| LCD SDA / SCL | GPIO 21 / GPIO 22 | Common ESP32 defaults added during cleanup; verify physically |

## Checks required before wiring or power-up

1. Confirm the exact ESP32 board model and pin labels.
2. Confirm the LCD backpack address with an I²C scanner; `0x27` is the address configured in the source, not a measured result.
3. Confirm whether the relay board is active-LOW. The supplied firmware treats LOW as ON and HIGH as OFF.
4. Confirm each relay channel is connected to the intended load.
5. If the ultrasonic module's ECHO output is 5 V, use a suitable voltage divider or level shifter so the ESP32 GPIO receives a 3.3 V-safe signal.
6. Use a suitable external supply for motors, pumps, heaters, and relay coils. Do not power high-current loads from an ESP32 GPIO or its 3.3 V output.
7. Keep mains-voltage wiring isolated and protected; use correctly rated enclosures, fuses, and qualified supervision.
8. GPIO 12 and GPIO 15 are ESP32 strapping pins on common ESP32 variants. Verify the circuit does not interfere with boot configuration.

Do not treat this table as a substitute for checking the actual prototype wiring.
