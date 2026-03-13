#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include "config.h"

// ── LED setup ────────────────────────────────────────────────────────────────
CRGB leds[NUM_LEDS];

// ── Light state ──────────────────────────────────────────────────────────────
struct LightState {
  bool     on        = false;
  uint8_t  bri       = 254;
  uint16_t hue       = 8418;   // warm white
  uint8_t  sat       = 140;
  uint16_t ct        = 370;
  String   colormode = "hs";
} state;

WebServer server(80);

// ── Apply current state to LED matrix ────────────────────────────────────────
void applyToMatrix() {
  if (!state.on) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  } else if (state.colormode == "ct") {
    // Colour temperature: map ct (153 cool – 500 warm) to white
    uint8_t r = state.bri;
    uint8_t g = state.bri;
    uint8_t b = (uint8_t)map(state.ct, 153, 500, state.bri, state.bri / 4);
    fill_solid(leds, NUM_LEDS, CRGB(r, g, b));
  } else {
    // HS mode
    CHSV hsv(
      (uint8_t)map(state.hue, 0, 65535, 0, 255),
      state.sat,
      state.bri
    );
    CRGB color;
    hsv2rgb_rainbow(hsv, color);
    fill_solid(leds, NUM_LEDS, color);
  }
  FastLED.show();
}

// ── Build JSON responses ──────────────────────────────────────────────────────
String lightJson() {
  JsonDocument doc;
  JsonObject s = doc["state"].to<JsonObject>();
  s["on"]        = state.on;
  s["bri"]       = state.bri;
  s["hue"]       = state.hue;
  s["sat"]       = state.sat;
  s["ct"]        = state.ct;
  s["colormode"] = state.colormode;
  s["effect"]    = "none";
  s["alert"]     = "none";
  s["reachable"] = true;
  doc["type"]            = "Extended color light";
  doc["name"]            = LIGHT_NAME;
  doc["modelid"]         = "LCT015";
  doc["manufacturername"]= "Philips";
  doc["productname"]     = "Hue color lamp";
  doc["uniqueid"]        = LIGHT_UID;
  doc["swversion"]       = "1.90.1";
  String out; serializeJson(doc, out); return out;
}

// ── HTTP handlers ─────────────────────────────────────────────────────────────
void handleGetLight() {
  server.send(200, "application/json", lightJson());
}

void handlePutState() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "[]");
    return;
  }
  JsonDocument req;
  deserializeJson(req, server.arg("plain"));

  JsonDocument resp;
  JsonArray arr = resp.to<JsonArray>();

  auto ok = [&](const char* key, auto val) {
    String path = String("/lights/1/state/") + key;
    JsonObject s = arr.add<JsonObject>();
    s["success"][path] = val;
  };

  if (req.containsKey("on"))  { state.on  = req["on"];  ok("on",  state.on);  }
  if (req.containsKey("bri")) { state.bri = req["bri"]; ok("bri", state.bri); state.colormode = "hs"; }
  if (req.containsKey("hue")) { state.hue = req["hue"]; ok("hue", state.hue); state.colormode = "hs"; }
  if (req.containsKey("sat")) { state.sat = req["sat"]; ok("sat", state.sat); state.colormode = "hs"; }
  if (req.containsKey("ct"))  { state.ct  = req["ct"];  ok("ct",  state.ct);  state.colormode = "ct"; }

  applyToMatrix();

  String out; serializeJson(resp, out);
  server.send(200, "application/json", out);
}

// UPnP description – needed for Hue bridge discovery
void handleDescription() {
  String mac = WiFi.macAddress(); mac.replace(":", "");
  String ip  = WiFi.localIP().toString();
  String xml =
    "<?xml version=\"1.0\"?>"
    "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
      "<specVersion><major>1</major><minor>0</minor></specVersion>"
      "<URLBase>http://" + ip + "/</URLBase>"
      "<device>"
        "<deviceType>urn:schemas-upnp-org:device:Basic:1</deviceType>"
        "<friendlyName>" LIGHT_NAME "</friendlyName>"
        "<manufacturer>Royal Philips Electronics</manufacturer>"
        "<modelName>Philips hue bridge 2012</modelName>"
        "<modelNumber>929000226503</modelNumber>"
        "<serialNumber>" + mac + "</serialNumber>"
        "<UDN>uuid:2f402f80-da50-11e1-9b23-" + mac + "</UDN>"
      "</device>"
    "</root>";
  server.send(200, "text/xml", xml);
}

void setupRoutes() {
  // Support any /api/<username>/lights/1[/state] pattern
  server.onNotFound([]() {
    String uri = server.uri();
    HTTPMethod m = server.method();
    if (uri.endsWith("/lights/1") && m == HTTP_GET)          { handleGetLight();  return; }
    if (uri.endsWith("/lights/1/state") && m == HTTP_PUT)    { handlePutState();  return; }
    if (uri == "/description.xml")                            { handleDescription(); return; }
    server.send(404, "application/json", "{}");
  });
}

// ── WiFi + startup animation ──────────────────────────────────────────────────
void connectWiFi() {
  Serial.printf("Connecting to %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    // Blink red while waiting
    leds[i % NUM_LEDS] = CRGB::Red; FastLED.show();
    delay(80);
    leds[i % NUM_LEDS] = CRGB::Black;
    i++; Serial.print(".");
  }
  fill_solid(leds, NUM_LEDS, CRGB::Black); FastLED.show();
  Serial.printf("\nIP: %s\n", WiFi.localIP().toString().c_str());
  // Green flash = connected
  fill_solid(leds, NUM_LEDS, CRGB(0, 80, 0)); FastLED.show(); delay(400);
  fill_solid(leds, NUM_LEDS, CRGB::Black);    FastLED.show();
}

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  fill_solid(leds, NUM_LEDS, CRGB::Black); FastLED.show();

  connectWiFi();
  setupRoutes();
  server.begin();

  Serial.printf("Hue light emulator running at http://%s\n", WiFi.localIP().toString().c_str());
  Serial.println("Register with bridge:");
  Serial.printf("  POST http://<bridge-ip>/api/<key>/lights  body: {\"deviceid\":[\"esp32matrix\"]}\n");
  Serial.printf("  or manually add via CLIP API\n");
}

void loop() {
  server.handleClient();
}
