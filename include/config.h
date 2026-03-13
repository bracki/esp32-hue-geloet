#pragma once

// WiFi
#define WIFI_SSID     "your-ssid"
#define WIFI_PASSWORD "your-password"

// Hue light identity (shown in Hue app)
#define LIGHT_NAME  "ESP32 LED Matrix"
#define LIGHT_UID   "00:17:88:01:00:be:ef:01-0b"  // unique, keep as-is or change last bytes

// LED Matrix (WS2812B 16x16)
#define LED_PIN        5
#define MATRIX_WIDTH   16
#define MATRIX_HEIGHT  16
#define NUM_LEDS       (MATRIX_WIDTH * MATRIX_HEIGHT)
#define LED_TYPE       WS2812B
#define COLOR_ORDER    GRB
#define BRIGHTNESS     80   // 0-255 global cap
