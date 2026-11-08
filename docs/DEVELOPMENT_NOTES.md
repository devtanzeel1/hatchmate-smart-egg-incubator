# Development Notes

## Selected source

The selected starting point was `PROJECT CODE/SAIM 2/BLYNK 2 oky.txt` from the supplied archive. It uses Blynk virtual pins V0, V1, and V2 for temperature, humidity, and ultrasonic distance.

The alternate `blynk with v3 & v4.txt` adds status datastreams. In that file, the V4 update is placed inside temperature-control branches and uses the temperature relay condition, despite its comment describing humidity/crod status. It was therefore not selected as the starting version.

## Important interpretation

The source archive contains multiple experimental drafts. The selected file is a starting point, not proof that every feature was physically completed or that the code is the exact firmware flashed to the final prototype.

## Review notes

- The selected source uses DHT11 on GPIO 15.
- Relay outputs are assigned to GPIO 33, 32, 25, 14, and 26.
- Ultrasonic TRIG/ECHO are assigned to GPIO 12 and 13.
- LCD address is configured as `0x27` and dimensions are 20×4.
- The source uses active-LOW relay commands.
- The original source includes real Wi-Fi and Blynk credentials. Those values were intentionally not copied into this repository.
- The original source attempts to write to LCD row index 4 even though a 20×4 LCD has row indices 0–3.
- The original source uses a 15-second tray-relay demonstration cycle; this must not be represented as a real incubation turning schedule.

## Next development stage

The next stage is to compare this cleaned candidate against the final physical wiring, compile it with the exact Arduino libraries, and test each sensor and relay independently with the high-current loads disconnected.
