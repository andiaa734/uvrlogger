
#include "Arduino.h"
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "EspSaveCrash.h"

#include "web.h"
#include "canopen.h"
#include "can.h"
#include "uvr16x2.h"
#include "esphome2can.h"
#include "nmt.h"
#include "sdo.h"
#include "od.h"
#include "co_time.h"
#include "co_server.h"

#define STASSID "Wunderland"
#define STAPSK "S0nja_1986"

const char *ssid = "sid";
const char *password = "secret";
const char *HOST_NAME = "UVRLOG";
RemoteDebug Debug;
EspSaveCrash SaveCrash;

can_frame frame;
can_frame r_frame;

char heart_beat_active = 0;
char sync_with_hass = 1;
char pdo_transmit_active = 1;
char time_producer = 1;
unsigned int hb_time = 0, producer_heart_beat = 1000, hass_sync_time = 0, hass_sync_intervall = 3000,
             pdo_time = 0, pdo_intervall = 1000, time_message_time = 0, time_message_intervall = 6000;

void setup()
{

  //Serial.begin(115200);

  ArduinoOTA
      .onStart([]()
               {
                 detachInterrupt(4);
                 String type;
                 if (ArduinoOTA.getCommand() == U_FLASH)
                   type = "sketch";
                 else // U_SPIFFS
                   type = "filesystem";

                 // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                 //  Serial.println("Start updating " + type);
               });

  ArduinoOTA.begin();

  canopen_setup();

  Debug.begin(HOST_NAME); // Initialize the WiFi server

  Debug.setResetCmdEnabled(true); // Enable the reset command

  Debug.showProfiler(true); // Profiler (Good to measure times, to optimize codes)
  Debug.showColors(true);   // Colors

  WiFi.mode(WIFI_STA);
  WiFi.setHostname(HOST_NAME);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {

    // only do WiFi.begin if not already connected
    delay(500);
  }

  time_producer = SetupTimefromNTP();
  setupHTTPClient();

  Web::start();

  if (canopen_state == INITIALIZATION) //Node initializes and send bootup messsgage and goes to pre operation state
  {
    heart_beat_active = 1;
    nmt_send_boot_up_message();
    delay(100);
    canopen_state = PRE_OPERATIONAL;
  }
}

void loop()
{

  ArduinoOTA.handle();

  Web::handleClient();

  Debug.handle();

  String lastCmd = Debug.getLastCommand();

  if (heart_beat_active)
  {
    can_frame hb_frame;
    if (millis() > hb_time + producer_heart_beat * 10)
    {
      hb_time = millis();
      if ((canopen_state == PRE_OPERATIONAL) ||
          (canopen_state == OPERATIONAL) ||
          (canopen_state == STOPPED))
      {
        nmt_send_heartbeat_message(hb_frame, canopen_state);
      }
    }
  }

  if (sync_with_hass)
  {

    if (millis() > hass_sync_time + hass_sync_intervall * 10)
    {
      hass_sync_time = millis();
      getTemperatureToOD();
    }
  }

  if (millis() > time_message_time + time_message_intervall * 10)
  {
    if (time_producer)
    {
      time_message_time = millis();
      sendCOTimestamp();
    }
    else
    {
      time_producer = SetupTimefromNTP();
    }
  }

  if (flag_received)
  {
    flag_received = 0;
    frame = irq_frm;
    irq_frm.id = 0;

    debugV("Frame ID: %X", frame.id);
    if (frame.id > 0)
    {
      for (int i = 0; i < frame.dlc; i++)
      {

        debugV("Data[%i]: %X", i, frame.data[i]);
      }
    }

    handleCOServer(frame);

    if (lastCmd == "connect")
    {

      connect(CANOPEN_NODE_ID);

      Debug.clearLastCommand();
    }

    if (lastCmd == "disconnect")
    {

      disconnect(CANOPEN_NODE_ID);

      Debug.clearLastCommand();
    }

    if (lastCmd == "time")
    {
      sendCOTimestamp();

      Debug.clearLastCommand();
    }

    /*   if (lastCmd == "readinlet")
  {

    readInlets();
    Debug.clearLastCommand();
  } */

    if (lastCmd == "readoutlet")
    {

      readOutlets();
      Debug.clearLastCommand();
    }

    if (lastCmd == "readblock")
    {
      /*     Outlet hkpumpe = NewOutlet(0x0);
    readBlock(hkpumpe.Mode); */

      Debug.clearLastCommand();
    }
    if (lastCmd == "clearcrash")
    {

      SaveCrash.clear();
      Debug.clearLastCommand();
    }

    if (lastCmd == "printcrash")
    {

      SaveCrash.print(Debug);

      Debug.clearLastCommand();
    }

    /*  if (lastCmd == "testreadod")
  {

    od_sub_index = 0x0;
    odindex = od_find_index(0x2512);
    debugV("Index found: %i", odindex);

    int len = od_find_data_length(odindex, od_sub_index);
    debugV("Len: %i", len);

    if (len > 0)
      od_read_data(odindex, od_sub_index, data_buffer, len);
    for (int i = 0; i < len; i++)
    {
      debugV("Databuffer: %x", data_buffer[i]);
    }
    Debug.clearLastCommand();
  }

  if (lastCmd == "testwriteod")
  {

    od_sub_index = 0x1;
    odindex = od_find_index(0x4E01);
    float tempr = 18.5;
    int16_t tempi = tempr * 10;
    char data_buf[4];

    std::copy(static_cast<const char *>(static_cast<const void *>(&tempi)),
              static_cast<const char *>(static_cast<const void *>(&tempi)) + sizeof tempi,
              data_buf);

    od_write_data(odindex, od_sub_index, data_buf, 2);

    Debug.clearLastCommand();
  } */
  }
}
