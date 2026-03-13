# esp32-hue-geloet 💡

ESP32 + 16×16 WS2812B LED-Matrix meldet sich **selbst** bei der Philips Hue Bridge an – einmal flashen, fertig. Die Matrix erscheint dann als steuerbare Hue-Lampe in der App.

## Wie es funktioniert

```
ESP32 startet
  → verbindet mit WLAN
  → meldet sich bei der Bridge: "Hallo, ich bin eine Lampe\!"
  → Bridge gibt ihm eine Lampen-ID
  → ESP32 fragt alle 500ms seinen eigenen Zustand ab
  → setzt An/Aus, Farbe, Helligkeit auf die 16×16 Matrix um
```

## Hardware

| Bauteil | Details |
|---|---|
| ESP32 Dev Board | z. B. WROOM-32 |
| 16×16 WS2812B Matrix | 256 LEDs, 5V |
| Datenpin | GPIO 5 (in config.h änderbar) |
| Stromversorgung | 5V / mind. 4A |

## Schnellstart

### 1. config.h ausfüllen

```cpp
#define WIFI_SSID      "dein-netz"
#define WIFI_PASSWORD  "dein-passwort"
#define HUE_BRIDGE_IP  "192.168.1.x"     // Bridge IP
#define HUE_API_KEY    "dein-api-key"    // siehe unten
#define LIGHT_NAME     "LED Matrix 16x16"
```

### 2. Hue API Key holen (einmalig)

```bash
# 1. Link-Taste auf der Bridge drücken
# 2. Dann sofort:
curl -X POST http://<bridge-ip>/api \
  -d '{"devicetype":"esp32#matrix"}'
# → gibt zurück: [{"success":{"username":"DEIN-API-KEY"}}]
```

### 3. Flashen

```bash
pio run --target upload && pio device monitor
```

### 4. Fertig\!

Beim ersten Start registriert sich der ESP32 automatisch bei der Bridge.  
Die Lampe erscheint unter dem konfigurierten Namen in der Hue App.

> Falls die automatische Registrierung nicht klappt (ältere Bridge-Firmware): siehe **Manuelle Registrierung** weiter unten.

---

## Startup-Animationen

| Animation | Bedeutung |
|---|---|
| 🔴 Roter Lauflicht | Verbindet mit WLAN |
| 🟢 Grüner Blitz | WLAN verbunden |
| 🔵 Blau blinken (3×) | Registriere bei Bridge |
| ⚪ Weißer Blitz | Registrierung erfolgreich |

---

## Manuelle Registrierung (Fallback)

Falls der ESP32 nicht automatisch erkannt wird:

```bash
# Option A: Lampe manuell anlegen
curl -X POST http://<bridge-ip>/api/<api-key>/lights \
  -H "Content-Type: application/json" \
  -d '{"deviceid":["esp32:matrix:16x16"]}'

# Option B: Licht-ID direkt im Code setzen (src/main.cpp, Zeile setup)
# myLightId = 5;  // deine ID eintragen
# saveLightId(5);
```

---

## Unterstützte Hue-Parameter

| Parameter | Beschreibung | Wertebereich |
|---|---|---|
| `on` | An / Aus | `true` / `false` |
| `bri` | Helligkeit | 1 – 254 |
| `hue` | Farbton | 0 – 65535 |
| `sat` | Sättigung | 0 – 254 |
| `ct` | Farbtemperatur | 153 (kalt) – 500 (warm) |

---

## Abhängigkeiten

- [FastLED](https://github.com/FastLED/FastLED) `^3.6`
- [ArduinoJson](https://arduinojson.org/) `^7.0`
- WiFi, HTTPClient, Preferences (ESP32 built-in)
