/* copyright 2016 Bert Melis
   License: MIT
*/

#include "WifiPrinter.h"

WifiPrinter::WifiPrinter() :
  _server(80),
  _ws("/ws"),
  _flagForReboot(false),
  _rebootCb(nullptr),
  _fwVersion{"\0"} {}

void WifiPrinter::begin() {
  _ws.onEvent(std::bind(&WifiPrinter::_onEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
  _server.addHandler(&_ws);
  _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });
  _server.on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/css", w3_css);
  });
  _server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "image/x-icon", favicon_ico_gz, favicon_ico_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  _server.on("/update", HTTP_POST, std::bind(&WifiPrinter::_handleUpdateRDY, this, std::placeholders::_1),
                                   std::bind(&WifiPrinter::_handleUpdate, this, std::placeholders::_1,
                                                                                std::placeholders::_2,
                                                                                std::placeholders::_3,
                                                                                std::placeholders::_4,
                                                                                std::placeholders::_5,
                                                                                std::placeholders::_6));
  _server.onNotFound(std::bind(&WifiPrinter::_onRequest, this, std::placeholders::_1));
  _timer.attach(statsFreq, &WifiPrinter::_timerCb, this);
  _server.begin();
}

void WifiPrinter::loop() {
  static uint32_t lastMillis = 0;
  if (millis() - lastMillis <= 10 * 1000UL) {
    lastMillis = millis();
    // send stats
  }
  if (_flagForReboot) {
    if (_rebootCb)
      _rebootCb();
    _ws.textAll("Rebooting...\n");
    _ws.closeAll();
    uint32_t timer = 0;
    while (timer < 1000) {  // delay some time for async tasks to complete before restarting eg. disconnecting mqtt
      yield();
      ++timer;
    }
    ESP.restart();
  }
}

void WifiPrinter::_handleUpdateRDY(AsyncWebServerRequest *request) {
  _flagForReboot = !Update.hasError();
  AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", _flagForReboot?"OK":"FAIL");
  response->addHeader("Connection", "close");
  request->send(response);
}

void WifiPrinter::_handleUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index) {
    Serial.printf("Update Start: %s\n", filename.c_str());
#if defined ESP8266
    Update.runAsync(true);
    size_t size = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
#elif defined ARDUINO_ARCH_ESP32
    size_t size = UPDATE_SIZE_UNKNOWN;
#endif
    if (!Update.begin(size)) {
      // Update.printError(Serial);
    }
  }
  if (!Update.hasError()) {
    if (Update.write(data, len) != len) {
      // Update.printError(Serial);
    }
  }
  if (final) {
    if (Update.end(true)) {
      // Serial.printf("Update Success: %uB\n", index+len);
    } else {
      // Update.printError(Serial);
    }
  }
}

void WifiPrinter::_onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["sdkVersion"] = ESP.getSdkVersion();
#ifdef ARDUINO_ARCH_ESP32
    json["coreVersion"] = "n/a on esp32";
#else
    json["coreVersion"] = ESP.getCoreVersion();
#endif
    json["fwVersion"] = _fwVersion;
    json["wpVersion"] = _wpVersion;
    size_t len = json.measureLength();
    AsyncWebSocketMessageBuffer* wsBuffer = _ws.makeBuffer(len);
    if (wsBuffer) {
      json.printTo(reinterpret_cast<char*>(wsBuffer->get()), len + 1);
      client->text(wsBuffer);
    }
  }
}

void WifiPrinter::_onRequest(AsyncWebServerRequest *request) {
  // Handle Unknown Request
  request->send(404);
}

size_t WifiPrinter::write(uint8_t str) {
  return write(&str, 1);
}

size_t WifiPrinter::write(const uint8_t* buffer, size_t size) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["message"] = buffer;
  size_t len = json.measureLength();
  AsyncWebSocketMessageBuffer* wsBuffer = _ws.makeBuffer(len);
  if (wsBuffer) {
    json.printTo(reinterpret_cast<char*>(wsBuffer->get()), len + 1);
    _ws.textAll(wsBuffer);
  }
  return len;
}

void WifiPrinter::_timerCb(WifiPrinter* wp) {
  if (wp) {
    wp->_updateStats();
  }
}

void WifiPrinter::_updateStats() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["freeHeap"] = ESP.getFreeHeap();
  json["uptime"] = millis();
  size_t len = json.measureLength();
  AsyncWebSocketMessageBuffer* wsBuffer = _ws.makeBuffer(len);
  if (wsBuffer) {
    json.printTo(reinterpret_cast<char*>(wsBuffer->get()), len + 1);
    _ws.textAll(wsBuffer);
  }
}
