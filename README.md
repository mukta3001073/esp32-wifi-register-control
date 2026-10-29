# ESP32 Wi-Fi & Register Control

An IoT telemetry node simulated in Wokwi (ESP32 DevKit V1). It reads a potentiometer, prints the raw 12-bit value to the Serial Monitor, sends it over Wi-Fi with an HTTP GET request every 3 seconds, and toggles a status LED using direct GPIO registers.

## Hardware (Wokwi)

| Component     | Pin                                  |
|---------------|--------------------------------------|
| Potentiometer | SIG -> GPIO 4, VCC -> 3V3, GND -> GND |
| Status LED    | GPIO 2 -> LED -> resistor -> GND      |

## Features

- Reads the analog sensor on GPIO 4 at 12-bit resolution (0-4095) and prints it to Serial
- Connects to Wokwi virtual Wi-Fi (SSID `Wokwi-GUEST`, empty password)
- Sends `GET http://httpbin.org/get?sensor=VAL` every 3 seconds and prints the HTTP status code
- Toggles the LED on GPIO 2 with `GPIO_OUT_W1TS_REG` (set) and `GPIO_OUT_W1TC_REG` (clear) instead of `digitalWrite()`

## Design note: ADC2 and Wi-Fi

GPIO 4 belongs to the ESP32's ADC2 unit. ADC2 is shared with the Wi-Fi driver, so while Wi-Fi is active `analogRead()` on GPIO 4 can return 0 or invalid values. In my Wokwi runs, a version that kept Wi-Fi on the whole time printed `Sensor: 0` even when the knob was turned.

To keep the reading correct on GPIO 4, the sketch does the following each 3-second cycle:

1. Toggle the LED through the registers
2. Turn Wi-Fi off briefly (~50 ms) and average 8 `analogRead()` samples
3. Turn Wi-Fi back on and reconnect
4. Send the HTTP GET request while Wi-Fi is active

Trade-off: Wi-Fi is not connected during the short sampling window. Moving the sensor to an ADC1 pin (for example GPIO 34) would avoid this, but the task specifies GPIO 4 (or 15).

## Files

- `sketch.ino` - firmware
- `diagram.json` - Wokwi circuit

## How to run

1. Open the project in Wokwi (ESP32 DevKit V1)
2. Paste `sketch.ino` and `diagram.json`
3. Start the simulation and open the Serial Monitor
4. Turn the potentiometer knob and watch `Sensor:` change; `HTTP code: 200` confirms the request went through
