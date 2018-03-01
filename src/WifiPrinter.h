/* copyright 2016 Bert Melis
   License: MIT
*/

#define WP_EXTENDED 0

#pragma once
#include <functional>
#include <Arduino.h>
#if defined ARDUINO_ARCH_ESP32
#include <Update.h>
#if WP_EXTENDED
#include <ESP32Ticker.h>
#endif
#elif defined ARDUINO_ARCH_ESP8266
#include <Updater.h>
#if WP_EXTENDED
#include <Ticker.h>
#endif
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
  void setFwVersion(const char* fwVersion) { strncpy(_fwVersion, fwVersion, 11); }
  virtual size_t write(uint8_t character);
  virtual size_t write(const uint8_t* buffer, size_t size);

 private:
  AsyncWebServer _server;
  AsyncWebSocket _ws;
  AsyncResponseStream* _response;
  bool _flagForReboot;
  RebootCb _rebootCb;  // can be made blocking
  char _fwVersion[12];

  void _onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
  void _onRequest(AsyncWebServerRequest *request);
  void _handleUpdateRDY(AsyncWebServerRequest *request);
  void _handleUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
#if WP_EXTENDED
// Exdented version
  Ticker _timer;
  bool _updateStats;
  static void _timerCb(WifiPrinter* wp);
  uint64_t _loopCounter;
  uint32_t _lastMicros;
#endif
};
