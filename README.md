# Update --> Redesign!!
I'm not maintaining the current code as I'm working on a completely new version.
The new version will be totally different:
- use the async webserver from @me-no-dev
- display printed messages via websockets on a status page
- incorporate an upload form for firmware updates
- display basis stats like uptime, free heap, firmware version etc...
This will be more close to my personal needs.

As this was only a slim version of the original version of @JoaoLopesF, which is still actively maintained, you can switch over to that version.
I will be changing the codebase completely in the coming weeks.

# WifiPrinter
[![Build Status](https://travis-ci.org/bertmelis/WifiPrinter.svg?branch=master)](https://travis-ci.org/bertmelis/WifiPrinter)

Arduino library for ESP8266 to print debug messages on a telnet-like server.

Adds the ability to use a telnet-like server to print debug messages on when your Serial0 interface is occupied by something else or when you cannot physically reach your device.

## Credits go to
- (C) 2015 Rudolf Reuter http://www.rudiswiki.de/wiki9 --> original work
- (C) 2016 JoaoLopesF https://github.com/JoaoLopesF --> RemoteDebug library
- (C) 2017 Bert Melis https://github.com/bertmelis --> stripped down version
