#ifndef ESPHOME2CAN_H
#define ESPHOME2CAN_H

#include "ArduinoJson.h"
#include <ESP8266WiFi.h>
#include "RemoteDebug.h"

#include <ESP8266HTTPClient.h>
#include "WiFiClient.h"

const String bearer_token = "Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiOiI5MjJhMGQ4NTNlYTI0ZjUwYmUxZDdmZmIyZmQyOTZkZSIsImlhdCI6MTYzNDc2MjQ0MywiZXhwIjoxOTUwMTIyNDQzfQ.HB6NzRI0tcF7Jw1xsnmp5l98xO4KcY70HFDCOAUeeQg";


extern RemoteDebug Debug;


DynamicJsonDocument getRoomTemperature();
void setupHTTPClient();
void getTemperatureToOD();


#endif