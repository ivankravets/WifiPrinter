# WifiPrinter

[![Build Status](https://travis-ci.org/bertmelis/WifiPrinter.svg?branch=master)](https://travis-ci.org/bertmelis/WifiPrinter)

Arduino library for ESP8266 and ESP32 to print on a device-hosted webpage via websockets.

You can also use an "extended mode" (by changing `WP_EXTENDED` to `1` in `WifiPrinter.h`). Then you can also see some basic data like uptime, free heap memory and the average Arduino loop() duration.

I only tested the UI in Google Chrome. IE and Edge work but have UI issues.

## Usage

* include the header in your sketch  
`#include <WifiPrinter.h>`  
* create an instance of the WifiPrinter (and name it as you like).  
`WifiPrinter wifiPrinter;`
* optionally set your firmware version (in `void setup()`)  
`wifiPrinter.setFirmware("1.5.1");`
* start the server  
`wifiPrinter.begin();`
* print messages like you would do on the Serial interface  
`wifiPrinter.println("Hello World!");`  
* don't forget to include to `wifiPrinter.loop()` in your main loop  

For a complete version see the example.

**Warning:** the webserver is completely unprotected. Do not use in public environments!

Developement takes place in the `redesign` branch.
If you wanted to have the old lib, please visit the [RemoteDebug repo](https://github.com/JoaoLopesF/RemoteDebug) or switch to the (soon to be overwritten) master branch.