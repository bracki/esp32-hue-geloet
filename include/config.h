#pragma once

// WiFi
#define WIFI_SSID      "your-ssid"
#define WIFI_PASSWORD  "your-password"

// Hue Bridge – IP und API-Key eintragen
// API-Key erstellen: Link-Taste drücken, dann:
//   curl -X POST http://<bridge-ip>/api -d '{"devicetype":"esp32#matrix"}'
#define HUE_BRIDGE_IP  "192.168.1.x"
#define HUE_API_KEY    "your-api-key-here"

// Name der Lampe in der Hue App
#define LIGHT_NAME     "LED Matrix 16x16"

// LED Matrix
#define LED_PIN        5
#define MATRIX_WIDTH   16
#define MATRIX_HEIGHT  16
#define NUM_LEDS       (MATRIX_WIDTH * MATRIX_HEIGHT)
#define LED_TYPE       WS2812B
#define COLOR_ORDER    GRB
#define BRIGHTNESS     80
