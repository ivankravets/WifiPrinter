#include "WifiPrinter.h"


WifiPrinter::WifiPrinter():
  _wifiServer(23),
  _buffer(""),
  _connected(false),
  _resetCommandEnabled(false),
  _command(""),
  _newLine(true),
  _showTime(false)
  {
    _buffer.reserve(200);
  }
WifiPrinter::WifiPrinter(uint16_t port):
  _wifiServer(port),
  _buffer(""),
  _connected(false),
  _resetCommandEnabled(false),
  _command(""),
  _newLine(true),
  _showTime(false)
  {
    _buffer.reserve(200);
  }


void WifiPrinter::begin() {
  _wifiServer.begin();
  _wifiServer.setNoDelay(true);
  while(_wifiClient.available())
    _wifiClient.read(); //clear input buffer
}


void WifiPrinter::stop() {
  if (_wifiClient && _wifiClient.connected()) _wifiClient.stop();
  _wifiServer.stop();
}


void WifiPrinter::loop() {
  //Handle connections
  if (_wifiServer.hasClient()) {
    if (!_wifiClient || !_wifiClient.connected()) {
      if (_wifiClient) _wifiClient.stop(); //stop second client, only 1 supported
      _wifiClient = _wifiServer.available();
      _wifiClient.flush(); //start with clean sheet
      _showInfo();
      //empty input buffer
      while(_wifiClient.available()) {
        _wifiClient.read();
      }
    }
  }
  //Loop through activities
  _connected = (_wifiClient && _wifiClient.connected());
  if (_connected) {
    //check if commands has been sent
    while(_wifiClient.available()) {
      char character = _wifiClient.read();
      if (character == '\n' || character == '\r') {
        //Commands end with <newline> --> handle command
        if (_command.length() > 0) {
          _handle();
        }
        _command = ""; // Init for next command
      } else if (isPrintable(character)) {
        // Concat
        _command.concat(character);
      }
    }
  }
}


// Allow ESP reset over telnet client
void WifiPrinter::enableReset(boolean enable){
    _resetCommandEnabled = enable;
}


boolean WifiPrinter::isActive() {
    return _connected;
}


size_t WifiPrinter::write(uint8_t character) {
  // New line writted before ?
  if (_newLine) {
    String show = "";
    // Show time in millis
    if (_showTime) {
      show.concat ("(t: ");
      show.concat (millis());
      show.concat ("ms)");
      if (_connected) {  // send data to Client
        _buffer = show;
      }
    }
    _newLine = false;
  }

  // Print ?
  boolean doPrint = false;

  // New line ?
  if (character == '\n') {
    _newLine = true;
    doPrint = true;
  } else if (_buffer.length() == 100) { // Limit of buffer
    doPrint = true;
  }

  // Write to telnet Buffered
  _buffer.concat((char)character);

  // Send the characters buffered by print.h
  if (doPrint) { // Print the buffer
    // Write to telnet Buffered
    if (_connected) {  // send data to Client
      _wifiClient.print(_buffer);
    }
    // Empty the buffer
    _buffer = "";
    return 1;
  } else return 1;
}

////// Private

// Show help of commands
void WifiPrinter::_showInfo() {
    // Show the initial message
    String help = "";
    help.concat(F("* ESP8266 WifiPrinter\r\n"));
    help.concat(F("* Commands:\r\n"));
    help.concat(F("   ? or help -> display these help of commands\r\n"));
    help.concat(F("   t -> display timestamps\r\n"));
    help.concat(F("   m -> display memory available\r\n"));
//    help.concat(F("   i -> display crash info\r\n"));
//    help.concat(F("   c -> clear crash info\r\n"));
    help.concat(F("   q -> quit (close this connection)\r\n"));
    if (_resetCommandEnabled) {
        help.concat(F("   reset -> reset the ESP8266\r\n"));
    }
    help.concat(F("* Please type the command and press enter to execute. (? or h for this help)\r\n\r\n"));
    _wifiClient.print(help);
}


// Process user command over telnet
void WifiPrinter::_handle() {
    _wifiClient.print(F("Command received: "));
    _wifiClient.println(_command);
    String options = "";
    uint8_t pos = _command.indexOf(" ");
    if (pos > 0) {
        options = _command.substring (pos+1);
    }
    // Process the command
    if (_command == "h" || _command == "?") {
        // Show help
        _showInfo();
    } else if (_command == "q") {
        // Quit
        _wifiClient.println(F("Closing telnet connection ..."));
        _wifiClient.stop();
    } else if (_command == "m") {
        _wifiClient.print(F("* Free Heap RAM: "));
        _wifiClient.println(ESP.getFreeHeap());
/*    } else if (_command == "i") {
      //display crash info
      //_wifiClient.println(F("* Display crash info"));
      SaveCrash.print(_wifiClient);
    } else if (_command == "c") {
      //clear crash info
      SaveCrash.clear();
      _wifiClient.println(F("* Crash info cleared.")); */
    }  else if (_command == "t") {
        // Show time
        _showTime = !_showTime;
        _wifiClient.printf("* Show time: %s\r\n", (_showTime)?"On":"Off");
    }  else if (_command == "reset" && _resetCommandEnabled) {
        _wifiClient.println(F("* Reset ..."));
        _wifiClient.println(F("* Closing telnet connection ..."));
        _wifiClient.println(F("* Resetting the ESP8266 ..."));
        _wifiClient.stop();
        _wifiServer.stop();
        delay (500);
        // Reset
        ESP.reset();
    }
}
