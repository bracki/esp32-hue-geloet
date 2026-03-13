#pragma once
#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "config.h"

// Returns the light ID on the bridge (stored in NVS), or -1 if not registered yet
int getSavedLightId() {
  Preferences prefs;
  prefs.begin("hue", true);
  int id = prefs.getInt("lightId", -1);
  prefs.end();
  return id;
}

void saveLightId(int id) {
  Preferences prefs;
  prefs.begin("hue", false);
  prefs.putInt("lightId", id);
  prefs.end();
}

// Try to find our light on the bridge by name
int findLightByName(const String& name) {
  HTTPClient http;
  http.begin(String("http://") + HUE_BRIDGE_IP + "/api/" + HUE_API_KEY + "/lights");
  int code = http.GET();
  if (code != 200) { http.end(); return -1; }

  JsonDocument doc;
  deserializeJson(doc, http.getString());
  http.end();

  for (JsonPair kv : doc.as<JsonObject>()) {
    if (String(kv.value()["name"].as<const char*>()) == name) {
      return String(kv.key().c_str()).toInt();
    }
  }
  return -1;
}

// Register ESP32 as a new light on the bridge.
// Returns light ID on success, -1 on failure.
// NOTE: Press the link button on the bridge before first call if API key is new.
int registerWithBridge(const String& myIP) {
  Serial.println("[Hue] Registering with bridge at " HUE_BRIDGE_IP "...");

  // 1. Trigger new-light search on bridge
  HTTPClient http;
  http.begin(String("http://") + HUE_BRIDGE_IP + "/api/" + HUE_API_KEY + "/lights");
  http.addHeader("Content-Type", "application/json");
  // Tell the bridge about us – some bridge versions support HTTP-based light registration
  String body = "{\"deviceid\":[\"esp32:matrix:16x16\"]}";
  int code = http.POST(body);
  String resp = http.getString();
  http.end();
  Serial.printf("[Hue] Light search response (%d): %s\n", code, resp.c_str());

  // 2. Wait a moment, then check if bridge found us by name
  delay(3000);
  int id = findLightByName(LIGHT_NAME);
  if (id > 0) {
    Serial.printf("[Hue] Found as light #%d\n", id);
    return id;
  }

  // 3. Fallback: Create a virtual light entry via CLIP v2 (newer bridges)
  http.begin(String("http://") + HUE_BRIDGE_IP + "/clip/v2/resource/light");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("hue-application-key", HUE_API_KEY);
  String v2body = String("{")
    + "\"metadata\":{\"name\":\"" + LIGHT_NAME + "\",\"archetype\":\"sultan_bulb\"},"
    + "\"on\":{\"on\":false},"
    + "\"dimming\":{\"brightness\":0},"
    + "\"color\":{\"xy\":{\"x\":0.3127,\"y\":0.3290}}"
    + "}";
  code = http.POST(v2body);
  resp = http.getString();
  http.end();
  Serial.printf("[Hue] CLIP v2 register (%d): %s\n", code, resp.c_str());

  // Check again
  delay(1000);
  id = findLightByName(LIGHT_NAME);
  if (id > 0) {
    Serial.printf("[Hue] Registered as light #%d\n", id);
    return id;
  }

  Serial.println("[Hue] Could not auto-register. See README for manual step.");
  return -1;
}
