#ifndef JSON_API_H
#define JSON_API_H

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

namespace JsonAPI
{

  String sendJsonData();
  int readJsonData(String json_data);
  std::string utf16_to_utf8(std::u16string const &s);

}
#endif