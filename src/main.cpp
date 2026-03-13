#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include "config.h"
#include "hue_register.h"

// ── LED Matrix ────────────────────────────────────────────────────────────────
CRGB leds[NUM_LEDS];

// ── Current light state ───────────────────────────────────────────────────────
struct LightState {
  bool     on        = false;
  uint8_t  bri       = 254;
  uint16_t hue       = 8418;
  uint8_t  sat       = 140;
  uint16_t ct        = 370;
  String   colormode = "hs";
} state;

int myLightId = -1;

// ── Apply state to LED matrix ─────────────────────────────────────────────────
void applyToMatrix() {
  if (!state.on) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  } else if (state.colormode == "ct") {
    uint8_t warmth = (uint8_t)map(state.ct, 153, 500, 255, 80);
    fill_solid(leds, NUM_LEDS, CRGB(state.bri, (uint8_t)(state.bri * warmth / 255), (uint8_t)(state.bri * (255 - warmth) / 512)));
  } else {
    CHSV hsv((uint8_t)map(state.hue, 0, 65535, 0, 255), state.sat, state.bri);
    CRGB color; hsv2rgb_rainbow(hsv, color);
    fill_solid(leds, NUM_LEDS, color);
  }
  FastLED.show();
}

// ── Poll own state from Hue bridge ────────────────────────────────────────────
void pollBridgeState() {
  if (myLightId < 0) return;
  HTTPClient http;
  String url = String("http://") + HUE_BRIDGE_IP + "/api/" + HUE_API_KEY + "/lights/" + myLightId;
  http.begin(url);
  if (http.GET() != 200) { http.end(); return; }

  JsonDocument doc;
  if (deserializeJson(doc, http.getString())) { http.end(); return; }
  http.end();

  JsonObject s = doc["state"];
  bool newOn  = s["on"]  | state.on;
  uint8_t  newBri = s["bri"] | state.bri;
  uint16_t newHue = s["hue"] | state.hue;
  uint8_t  newSat = s["sat"] | state.sat;
  uint16_t newCt  = s["ct"]  | state.ct;
  String   newCM  = s["colormode"] | state.colormode;

  // Only redraw if something changed
  if (newOn != state.on || newBri != state.bri || newHue != state.hue ||
      newSat != state.sat || newCt != state.ct || newCM != state.colormode) {
    state.on = newOn; state.bri = newBri; state.hue = newHue;
    state.sat = newSat; state.ct = newCt; state.colormode = newCM;
    Serial.printf("[LED] %s  bri=%d  hue=%d  sat=%d  ct=%d  mode=%s\n",
      state.on ? "ON" : "OFF", state.bri, state.hue, state.sat, state.ct, state.colormode.c_str());
    applyToMatrix();
  }
}

// ── WiFi ──────────────────────────────────────────────────────────────────────
void connectWiFi() {
  Serial.printf("[WiFi] Connecting to %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    leds[i % NUM_LEDS] = CRGB::Red; FastLED.show();
    delay(100);
    leds[i % NUM_LEDS] = CRGB::Black;
    i++; Serial.print(".");
  }
  fill_solid(leds, NUM_LEDS, CRGB::Black); FastLED.show();
  Serial.printf("\n[WiFi] IP: %s\n", WiFi.localIP().toString().c_str());
  // Green flash
  fill_solid(leds, NUM_LEDS, CRGB(0, 60, 0)); FastLED.show(); delay(300);
  fill_solid(leds, NUM_LEDS, CRGB::Black);    FastLED.show();
}

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  fill_solid(leds, NUM_LEDS, CRGB::Black); FastLED.show();

  connectWiFi();

  // Check if we already have a registered light ID
  myLightId = getSavedLightId();
  if (myLightId < 0) {
    Serial.println("[Hue] Not registered yet – trying to register with bridge...");
    // Blue pulse = registering
    for (int i = 0; i < 3; i++) {
      fill_solid(leds, NUM_LEDS, CRGB(0, 0, 60)); FastLED.show(); delay(300);
      fill_solid(leds, NUM_LEDS, CRGB::Black);    FastLED.show(); delay(200);
    }
    myLightId = registerWithBridge(WiFi.localIP().toString());
    if (myLightId > 0) {
      saveLightId(myLightId);
      Serial.printf("[Hue] Registered as light #%d – saved to flash\n", myLightId);
      // White flash = success
      fill_solid(leds, NUM_LEDS, CRGB(80, 80, 80)); FastLED.show(); delay(500);
      fill_solid(leds, NUM_LEDS, CRGB::Black);       FastLED.show();
    } else {
      Serial.println("[Hue] Registration failed. See README.");
    }
  } else {
    Serial.printf("[Hue] Already registered as light #%d\n", myLightId);
  }
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  pollBridgeState();
  delay(500);  // Poll every 500ms for snappy response
}
