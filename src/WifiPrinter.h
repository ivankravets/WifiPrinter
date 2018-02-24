/* copyright 2016 Bert Melis
   License: MIT
*/

#pragma once
#include <functional>
#include <Arduino.h>
#if defined ARDUINO_ARCH_ESP32
#include <Update.h>
#include <ESP32Ticker.h>
#elif defined ESP8266
#include <Updater.h>
#include <Ticker.h>
#else
#error Platform not supported
#endif

#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include "html.h"

#define _wpVersion "0.1.0"
#define statsFreq 1

typedef void (*RebootCb)();

class WifiPrinter : public Print {
 public:
  WifiPrinter();
  void begin();
  void stop();
  void loop();
  void setRebootCb(RebootCb cb);
  void setFwVersion(const char* fwVersion) { strncpy(_fwVersion, fwVersion, 11); };
  virtual size_t write(uint8_t character);
  virtual size_t write(const uint8_t* buffer, size_t size);
 private:
  AsyncWebServer _server;
  AsyncWebSocket _ws;
  Ticker _timer;
  AsyncResponseStream* _response;
  bool _flagForReboot;
  RebootCb _rebootCb; // can be made blocking
  char _fwVersion[12];

 private:
  void _onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
  void _onRequest(AsyncWebServerRequest *request);
  void _handleUpdateRDY(AsyncWebServerRequest *request);
  void _handleUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
  static void _timerCb(WifiPrinter* wp);
  void _updateStats();
};
