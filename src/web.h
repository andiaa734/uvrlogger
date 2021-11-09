#ifndef WEB_H
#define WEB_H

#include <ESP8266WebServer.h>
#include "ArduinoJson.h"
#include "uvr16x2.h"


/**
 * uvr2web (www.uvr2web.de)
 * =======
 *
 * Hochladen der Datenrahmen ins Internet via Ethernet
 * Elias Kuiter (2018)
 */

namespace Web
{

  void start(); // stellt eine Internetverbindung her
  void handleClient();
  void handleRoot();
  void handleForm();
  void handleDisconnect();
  void datatojson();
  void readJSON();
  std::string utf16_to_utf8(std::u16string const& s);


}
#endif