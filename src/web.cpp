#include "Arduino.h"
#include <web.h>
#include "can.h"
#include "web_pages.h"
#include "json_api.h"

char nodeid;
uint32_t rescobid;
bool connected = false;
bool readblock = false;

namespace Web
{

  ESP8266WebServer server(80);

  void start()
  {

    server.on("/", handleRoot);
    server.on("/disconnect", handleDisconnect);
    server.on("/get", handleForm); //form action is handled here
    server.on("/json", HTTP_GET, sendJSON);
    server.on("/json", HTTP_POST, readJSON);

    server.begin();
  }

  void handleDisconnect()
  {
    disconnect(nodeid);
  }

  void handleClient()
  {
    server.handleClient();
  }

  void handleRoot()
  {
    server.send(200, "text/html", index_html); //Send web page
  }

  void sendJSON()
  {
    String jsondata = JsonAPI::sendJsonData();

    if (jsondata != "")
    {
      server.send(200, "application/json;charset=utf-8", jsondata);
    }
    else {
      server.send(503, "text/plain");
    }
  }

  void readJSON()
  {

    int http_code = 0;

    if (server.args() > 0)
    {

      http_code = JsonAPI::readJsonData(server.arg("plain"));

      if (http_code == 200)
      {

        server.send(http_code, "application/json;charset=utf8", "");
      }

      if (http_code == 500)
      {

        server.send(http_code, "application/json;charset=utf8", "Invalid Json request");
      }
    }

    else
    {
      server.send(http_code, "application/json;charset=utf8", "Empty request");
    }
  }

  void handleForm()
  {

    can_frame frame;

    nodeid = strtoul(server.arg("nodeid").c_str(), nullptr, 8);
    rescobid = strtoul(server.arg("rescobid").c_str(), nullptr, 16);
    frame.id = strtoul(server.arg("cobid").c_str(), nullptr, 16) + nodeid;
    frame.dlc = server.arg("dlc").toInt();
    frame.extended = 0;
    frame.remote = 0;

    frame.data[0] = strtoul(server.arg("b0").c_str(), nullptr, 16);
    frame.data[1] = strtoul(server.arg("b1").c_str(), nullptr, 16);
    frame.data[2] = strtoul(server.arg("b2").c_str(), nullptr, 16);
    frame.data[3] = strtoul(server.arg("b3").c_str(), nullptr, 16);
    frame.data[4] = strtoul(server.arg("b4").c_str(), nullptr, 16);
    frame.data[5] = strtoul(server.arg("b5").c_str(), nullptr, 16);
    frame.data[6] = strtoul(server.arg("b6").c_str(), nullptr, 16);
    frame.data[7] = strtoul(server.arg("b7").c_str(), nullptr, 16);

    if (server.arg("crc") == "on")
    {

      uint8_t len = server.arg("crclen").toInt();

      uint16_t crc = crc16.crcarc(frame.data, 1, len);
      frame.data[len + 1] = crc & 0xff;
      frame.data[len + 2] = (crc >> 8);
    }

    if (server.arg("write") == "on")
    {
      uint8_t len = server.arg("crclen").toInt();
      char data_buffer[CANOPEN_MAX_DATA_BUFFER_LENGTH];
      data_buffer[0] = frame.data[4];
      data_buffer[1] = frame.data[5];
      data_buffer[2] = frame.data[6];
      data_buffer[3] = frame.data[7];

      ObjectIndex index;
      index.idx.b0 = frame.data[2];
      index.idx.b1 = frame.data[1];
      index.SubIndex = frame.data[3];
      setState(index, data_buffer, len);
    }

    if (server.arg("blockwrite") == "on")
    {
      uint8_t len = server.arg("crclen").toInt();
      char data_buffer[CANOPEN_MAX_DATA_BUFFER_LENGTH];
      data_buffer[0] = frame.data[4];
      data_buffer[1] = frame.data[5];
      data_buffer[2] = frame.data[6];
      data_buffer[3] = frame.data[7];

      ObjectIndex index;
      index.idx.b0 = frame.data[2];
      index.idx.b1 = frame.data[1];
      index.SubIndex = frame.data[3];
      writeBlock(index, (byte *)data_buffer, len);
    }

    for (int i = 0; i < 7; i++)
    {

      debugV("Send Data [%i]: %X", i, frame.data[i]);
    }

    if (server.arg("blockread") == "on")
    {
      byte data_buffer[CANOPEN_MAX_DATA_BUFFER_LENGTH];
      ObjectIndex index;
      byte length;
      index.idx.b0 = frame.data[1];
      index.idx.b1 = frame.data[2];
      index.SubIndex = frame.data[3];
      readBlock(index, data_buffer, &length);

      if (server.arg("string") == "on")
      {
        std::string block = readStringfromIndex(data_buffer, &length);
        debugV("Result: %s", block.c_str());
      }

      if (server.arg("int") == "on")
      {
        int block = readintfromIndex(data_buffer, &length);
        debugV("Result: %i", block);
      }

      if (server.arg("float") == "on")
      {
        float block = readfloatfromIndex(data_buffer, &length);
        debugV("Result: %f", block);
      }
    }

    if (server.arg("singleframe") == "on")

    {

      can_send_frame(frame, rescobid);
      wait(rescobid, frame);
    }

    server.send(200, "text/html", index_html); //Send web page
  }

}