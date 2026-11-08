# Safety, Known Limitations, and Validation Checklist

## Verification status

- The source was inspected and refactored for readability, credential separation, display indexing, and non-blocking scheduling.
- The edited firmware has **not** been compiled against the user's exact Arduino core and library versions.
- The edited firmware has **not** been flashed to or tested on the physical HatchMate prototype.
- Sensor calibration, relay polarity, actuator wiring, thermal performance, and incubation outcomes are unverified.

## Behaviour inherited from the supplied source

- Relay outputs are assumed active-LOW.
- Heater relay is driven ON below 36 °C and is also driven ON in the 36–39 °C band; it is driven OFF above 39 °C. The old LCD message contradicted the relay command in the middle band; the cleaned LCD now reports the commanded state.
- The humidity-related relay is ON below 50% relative humidity and OFF at or above 50% in the supplied version. The original `>55` branch did not create a true hysteresis band.
- Fan relay is ON when relative humidity is above 56% OR temperature is above 40 °C.
- Pump relay is ON when the measured ultrasonic distance is above 20 cm.
- Tray relay is ON during seconds 10–15 of each 15-second cycle. This is only a short demo/bench-test pulse, not a validated egg-turning schedule.
- Blynk virtual pins are V0 temperature, V1 humidity, and V2 water distance.

These thresholds and timings are copied from the supplied prototype code; they are not recommendations for egg incubation and have not been validated.

## Changes made in the cleaned candidate

- Removed embedded Wi-Fi credentials and the Blynk device token; credentials are loaded from a local `secrets.h` file.
- Added `secrets.example.h` and a `.gitignore` rule so the real `secrets.h` is not committed.
- Added an ultrasonic `pulseIn` timeout and treats a missing echo as invalid instead of converting it to 0 cm.
- Corrected LCD row usage to rows 0–3 for a 20×4 display; the original code attempted to write to row 4.
- Reworked the main loop to use `millis()` intervals instead of a blocking 2-second delay.
- Removed repeated LCD full-screen clears to reduce flicker.
- Kept the original relay thresholds, pin assignments, Blynk virtual pins, and short tray test pulse visible for review.

## Required validation before real use

- Regenerate the Blynk device token that appeared in the previously shared source. Do not reuse the exposed token.
- Change the Wi-Fi password if the previously shared password was real and is still in use.
- Confirm the new token and Wi-Fi details are stored only in local `secrets.h`.
- Verify relay active level with loads disconnected.
- Verify each GPIO-to-relay mapping against the actual wiring.
- Verify DHT11 readings against a reliable reference instrument.
- Verify ultrasonic distance readings at known distances and with the actual tank geometry.
- Verify LCD address and I²C pins.
- Confirm safe behaviour on DHT sensor failure, ultrasonic timeout, Wi-Fi loss, ESP32 reboot, and power interruption.
- Add an independent over-temperature cut-out and suitable electrical protection before any unattended heating operation.
- Replace the 15-second tray demo pulse with a properly researched and tested egg-turning schedule before using eggs.
- Test the complete system under supervision with dummy loads before connecting the actual heater, pump, or motor.

**Do not use this prototype as an unattended incubator or rely on it for animal welfare until the hardware and control logic have been independently validated.**
