/* copyright 2016 Bert Melis
   License: MIT
*/

#include <Arduino.h>
#if defined ARDUINO_ARCH_ESP32
#include <WiFi.h>
#elif defined ESP8266
#include <ESP8266WiFi.h>
#else
#error Platform not supported
#endif
#include <WifiPrinter.h>

const char* ssid = "xxxx";
const char* password = "xxxx";
const char* version = "1.0.0";

WifiPrinter wifiPrinter;

void setup() {
  Serial.begin(74880);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("STA: Failed!\n");
    WiFi.disconnect(false);
    delay(1000);
    WiFi.begin(ssid, password);
  }
  wifiPrinter.setFwVersion(version);
  wifiPrinter.begin();
}

void loop() {
  static uint32_t lastMillis = 0;
  if (millis() - lastMillis > 10000UL) {
    lastMillis = millis();
    wifiPrinter.println("Hi there, it's the ESP talking!");
  }
  wifiPrinter.loop();
}
