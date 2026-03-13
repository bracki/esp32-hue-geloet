# esp32-hue-geloet 💡

ESP32 + 16×16 WS2812B LED-Matrix als **Philips Hue Lampe** — die Matrix meldet sich bei der Hue Bridge an und lässt sich wie jede andere Hue-Lampe steuern.

## So funktioniert es

Der ESP32 läuft als **HTTP-Server**, der die Hue-Lampen-API emuliert. Die Hue Bridge schickt PUT-Requests an den ESP32 — dieser setzt Farbe, Helligkeit und An/Aus auf der LED-Matrix um.

```
Hue App → Bridge → PUT /api/<key>/lights/1/state → ESP32 → LED Matrix
```

## Hardware

| Bauteil | Details |
|---|---|
| ESP32 Dev Board | z. B. WROOM-32 |
| LED Matrix | 16×16 WS2812B (256 LEDs) |
| Datenpin | GPIO 5 (konfigurierbar) |
| Stromversorgung | 5V / min. 4A für volle Helligkeit |

## Setup

### 1. Config anpassen

`include/config.h` bearbeiten:

```cpp
#define WIFI_SSID     "dein-netzwerk"
#define WIFI_PASSWORD "dein-passwort"
#define LIGHT_NAME    "LED Matrix"   // Name in der Hue App
```

### 2. Flashen

```bash
pio run --target upload
```

### 3. Bei der Hue Bridge registrieren

Nach dem Start zeigt die serielle Konsole die IP des ESP32. Jetzt bei der Bridge anmelden:

```bash
# 1. Erstmal API-Key holen (Link-Button auf Bridge drücken\!)
curl -X POST http://<bridge-ip>/api \
  -d '{"devicetype":"esp32-hue-geloet#esp32"}'

# 2. ESP32 als Lampe hinzufügen
curl -X POST http://<bridge-ip>/api/<dein-api-key>/lights \
  -d '{"deviceid":["esp32:matrix:01"]}'
```

> Alternativ: Im Hue-App unter **Einstellungen → Meine Hue-Lampen → Lampe hinzufügen**

### 4. Licht manuell testen

```bash
# Einschalten, Farbe auf Rot
curl -X PUT http://<esp32-ip>/api/test/lights/1/state \
  -d '{"on":true,"hue":0,"sat":254,"bri":200}'

# Ausschalten
curl -X PUT http://<esp32-ip>/api/test/lights/1/state \
  -d '{"on":false}'
```

## Unterstützte Hue-Befehle

| Parameter | Beschreibung | Wertebereich |
|---|---|---|
| `on` | An/Aus | `true` / `false` |
| `bri` | Helligkeit | 1 – 254 |
| `hue` | Farbton | 0 – 65535 |
| `sat` | Sättigung | 0 – 254 |
| `ct` | Farbtemperatur (Weiß) | 153 (kalt) – 500 (warm) |

## Startup-Animation

- 🔴 Roter Lauflicht = verbindet mit WLAN
- 🟢 Grüner Blitz = verbunden, bereit

## Abhängigkeiten

- [FastLED](https://github.com/FastLED/FastLED)
- [ArduinoJson](https://arduinojson.org/)
- WebServer, WiFi (ESP32 built-in)
