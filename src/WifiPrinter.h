#pragma once
#include <Arduino.h>
#include <ESP8266WiFi.h>
//#include <EspSaveCrash.h>

/*
 *   This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 (C) 2015 Rudolf Reuter www.rudiswiki.de/wiki9 --> original work
 (C) 2016 JoaoLopesF https://github.com/JoaoLopesF --> RemoteDebug library
 (C) 2017 Bert Melis https://github.com/bertmelis --> stripped down version


 */


class WifiPrinter: public Print {
  public:
    WifiPrinter();
    WifiPrinter(uint16_t port);
    void begin();
    void stop();
    void loop();
    void enableReset(boolean enable);
    boolean isActive();
  	virtual size_t write(uint8_t);
  private:
    WiFiServer _wifiServer;
    WiFiClient _wifiClient;
    boolean _connected;
    boolean _resetCommandEnabled;
    String _buffer;
    String _command;				// Command received
    boolean _newLine;				// New line write?
    boolean _showTime;				// Show time in millis
    void _showInfo();
    void _handle();
};
