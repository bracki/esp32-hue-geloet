#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include "config.h"

CRGB leds[NUM_LEDS];

// Convert Hue bri (0-254) to FastLED brightness (0-255)
uint8_t hueBriToFastLED(int bri) {
  return map(bri, 0, 254, 0, 255);
}

// Convert Hue hue (0-65535) + sat (0-254) + bri (0-254) to CRGB
CRGB hueStateToRGB(int hue, int sat, int bri, bool on) {
  if (!on) return CRGB::Black;
  CHSV hsv(
    map(hue, 0, 65535, 0, 255),
    map(sat, 0, 254, 0, 255),
    hueBriToFastLED(bri)
  );
  CRGB rgb;
  hsv2rgb_rainbow(hsv, rgb);
  return rgb;
}

void connectWiFi() {
  Serial.printf("Connecting to %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
}

void fetchAndDisplayLights() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    return;
  }

  HTTPClient http;
  String url = String("http://") + HUE_BRIDGE_IP + "/api/" + HUE_API_KEY + "/lights";
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode != 200) {
    Serial.printf("HTTP error: %d\n", httpCode);
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    Serial.printf("JSON error: %s\n", err.c_str());
    return;
  }

  // Clear matrix
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  int ledIndex = 0;
  for (JsonPair kv : doc.as<JsonObject>()) {
    if (ledIndex >= NUM_LEDS) break;
    JsonObject state = kv.value()["state"];
    bool on   = state["on"]  | false;
    int  hue  = state["hue"] | 0;
    int  sat  = state["sat"] | 0;
    int  bri  = state["bri"] | 0;

    leds[ledIndex++] = hueStateToRGB(hue, sat, bri, on);
  }

  FastLED.show();
  Serial.printf("Updated %d lights on matrix\n", ledIndex);
}

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  connectWiFi();
}

void loop() {
  fetchAndDisplayLights();
  delay(POLL_INTERVAL);
}

