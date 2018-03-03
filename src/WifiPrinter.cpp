/* copyright 2016 Bert Melis
   License: MIT
*/

#include "WifiPrinter.h"

WifiPrinter::WifiPrinter() :
  _server(80),
  _ws("/ws"),
  _flagForReboot(false),
  _rebootCb(nullptr),
#if WP_EXTENDED
  _updateStats(false),
  _loopCounter(0),
  _lastMicros(0),
#endif
  _fwVersion{"\0"} {}

void WifiPrinter::begin() {
  _ws.onEvent(std::bind(&WifiPrinter::_onEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
  _server.addHandler(&_ws);
  _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    /*request->send_P(200, "text/html", index_html);*/
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html_gz, index_html_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
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
#if WP_EXTENDED
  _timer.attach(statsFreq, &WifiPrinter::_timerCb, this);
#endif
  _server.begin();
}

void WifiPrinter::loop() {
#if WP_EXTENDED
  ++_loopCounter;
  if (_updateStats) {
    _updateStats = false;
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["freeHeap"] = ESP.getFreeHeap();
    json["uptime"] = millis();
    json["avgLoop"] = (micros() - _lastMicros) / static_cast<float>(_loopCounter);
    _loopCounter = 0;
    _lastMicros = micros();
    size_t len = json.measureLength();
    AsyncWebSocketMessageBuffer* wsBuffer = _ws.makeBuffer(len);
    if (wsBuffer) {
      json.printTo(reinterpret_cast<char*>(wsBuffer->get()), len + 1);
      _ws.textAll(wsBuffer);
    }
  }
#endif
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
#if defined ARDUINO_ARCH_ESP8266
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
#if WP_EXTENDED
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
#endif
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
  char msg[200] = {"\0"};
  strncpy(msg, sizeof(msg), "{\"message\":\"");
  strncat(msg, sizeof(msg) - strlen(msg), buffer);
  strncat((msg, sizeof(msg) - strlen(msg), "\"");
  _ws.textAll(msg);
  return strlen(msg);
}

#if WP_EXTENDED
void WifiPrinter::_timerCb(WifiPrinter* wp) {
  if (wp) {
    wp->_updateStats = true;
  }
}
#endif
