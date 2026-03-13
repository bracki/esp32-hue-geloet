#pragma once

// WiFi
#define WIFI_SSID     "your-ssid"
#define WIFI_PASSWORD "your-password"

// Philips Hue
#define HUE_BRIDGE_IP  "192.168.1.x"
#define HUE_API_KEY    "your-hue-api-key"

// LED Matrix
#define LED_PIN        5
#define MATRIX_WIDTH   16
#define MATRIX_HEIGHT  16
#define NUM_LEDS       (MATRIX_WIDTH * MATRIX_HEIGHT)
#define LED_TYPE       WS2812B
#define COLOR_ORDER    GRB
#define BRIGHTNESS     50

// Polling interval (ms)
#define POLL_INTERVAL  2000

