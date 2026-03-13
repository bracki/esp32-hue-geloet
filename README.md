# esp32-hue-geloet

ESP32 mit einer 16x16 WS2812B LED-Matrix, gesteuert über die Philips Hue Bridge.

## Hardware

- ESP32 Dev Board
- 16x16 WS2812B LED Matrix (256 LEDs)
- Philips Hue Bridge (v2)

## Features

- Verbindung zur Hue Bridge via lokaler REST API
- Anzeige von Hue Lampen-Status auf der LED Matrix
- Farbe & Helligkeit werden von der Hue Bridge übernommen
- WiFi-Konfiguration via `config.h`

## Setup

1. `include/config.h` anpassen (SSID, Passwort, Hue Bridge IP + API Key)
2. Mit PlatformIO flashen: `pio run --target upload`

## Hue API Key erstellen

```bash
curl -X POST http://<bridge-ip>/api \
  -d '{"devicetype":"esp32-hue-geloet#esp32"}'
# Dann den Link-Button auf der Bridge drücken
```

## Abhängigkeiten

- [FastLED](https://github.com/FastLED/FastLED)
- [ArduinoJson](https://arduinojson.org/)
- WiFi (ESP32 built-in)
- HTTPClient (ESP32 built-in)
