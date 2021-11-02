
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

#define STASSID "Wunderland"
#define STAPSK "S0nja_1986"

const char *ssid = "Wunderland";
const char *password = "S0nja_1986";
const char *HOST_NAME = "UVRLOG";
RemoteDebug Debug;
EspSaveCrash SaveCrash;

can_frame frame;
can_frame r_frame;

char heart_beat_active = 0;
char sync_with_hass = 1;
char pdo_transmit_active = 1;
char canopen_state = INITIALIZATION, sdo_message_type, state_changed = 1, sdo_toggle;
unsigned int hb_time = 0, producer_heart_beat = 1000, hass_sync_time = 0, hass_sync_intervall = 3000, pdo_time = 0, pdo_intervall = 1000;
char data_buffer[CANOPEN_MAX_DATA_BUFFER_LENGTH];
unsigned char od_sub_index;
int od_index, odindex;
char number_of_segments, counter = 0;
char od_data_length;

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

  /*     if (pdo_transmit_active)
  {
    can_frame pdo_frame;
    if (millis() > pdo_time + pdo_intervall * 10)
    {
      pdo_time = millis();
      if ((canopen_state == PRE_OPERATIONAL) ||
          (canopen_state == OPERATIONAL))
       {
        pdo_transmit_data(RPDO_0_COMMUNICATION_PARAMETER + 0, RPDO_0_MAPPING_PARAMETER +0);
      }
    }
  } */

  if (sync_with_hass)
  {

    if (millis() > hass_sync_time + hass_sync_intervall * 10)
    {
      hass_sync_time = millis();
      getTemperatureToOD();
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

    uint16_t cob_id = frame.id & 0x780;

    switch (cob_id)
    {

    case 0x0:

      debugV("Broadcast");

      break;

    case RPDO_2_MESSAGE:
    {
      can_frame rframe;
      rframe.extended = 0;
      rframe.dlc = 8;
      rframe.id = RPDO_2_MESSAGE + CANOPEN_NODE_ID;

      if ((frame.data[0] & 0x80) == 0x80 && (frame.data[0] & 0x7F) == CANOPEN_NODE_ID)
      {

        byte client_id = frame.id & 0x3F;

        rframe.data[0] = 0x80 | client_id;
        rframe.data[1] = 0x80;
        rframe.data[2] = 0x12;
        rframe.data[3] = 0x01;
        rframe.data[4] = 0x40 + client_id;
        rframe.data[5] = 0x06;
        rframe.data[6] = 0x00;

        if (frame.data[1] == 0x00 &&
            frame.data[2] == 0x1F &&
            frame.data[3] == 0x00 &&
            frame.data[4] == CANOPEN_NODE_ID &&
            frame.data[5] <= 0x3f &&
            frame.data[6] == 0x80 &&
            frame.data[7] == 0x12)
        {

          rframe.data[7] = 0x00; //0x80 if deny

          can_send_frame(rframe);
        }

        else if (frame.data[1] == 0x01 &&
                 frame.data[2] == 0x1F &&
                 frame.data[3] == 0x00 &&
                 frame.data[5] <= 0x3f &&
                 frame.data[6] == 0x80 &&
                 frame.data[7] == 0x12)
        {
          if (frame.data[4] == CANOPEN_NODE_ID)
          {

            rframe.data[7] = 0x80;
            can_send_frame(rframe);
          }
        }
      }
    }
    break;

    case RSDO_MESSAGE:
    {
      can_frame rframe;
      byte client_id = frame.id & 0x3F;
      rframe.id = SSDOServerToClient2 + client_id;
      rframe.extended = 0;
      rframe.dlc = 8;

      byte command = frame.data[0];
      byte ccs = command & 0xE0;
      rframe.data[0] = INITIATE_SDO_UPLOAD_REQUEST | TRANSFER_SIZE_INDICATED;

      switch (ccs)
      {

      case INITIATE_SDO_UPLOAD_REQUEST:
      {
        od_index = frame.data[1] | (frame.data[2] << 8);
        od_sub_index = frame.data[3];
        odindex = od_find_index(od_index);
        if (odindex != -1)
        {
          od_data_length = od_find_data_length(odindex, od_sub_index);
          debugV("len: %i", od_data_length);
          if (od_data_length > 0)
          {
            od_read_data(odindex, od_sub_index, data_buffer, od_data_length);

            if (od_data_length <= 4)
            {
              debugV("RSDO Expedited upload for 0x%X:%d", od_index, od_sub_index);
              rframe.data[0] |= EXPEDITED_UPLOAD;
              rframe.data[0] |= (4 - od_data_length) << 2;
              rframe.data[1] = frame.data[1];
              rframe.data[2] = frame.data[2];
              rframe.data[3] = frame.data[3];
              rframe.data[4] = 0x00;
              rframe.data[5] = 0x0;
              rframe.data[6] = 0x0;
              rframe.data[7] = 0x0;

              for (int i = 0; i < od_data_length; i++)
              {
                rframe.data[i + 4] = data_buffer[i];
              }
              can_send_frame(rframe);
            }
            else
            {
              if (od_data_length > 4) //if data is more than 4 bytes do segmented transfer

              {
                counter = 0;
                sdo_toggle = 0;
                if (od_data_length % 7 == 0)
                {
                  number_of_segments = (od_data_length / 7); //no. of segments = data length/7 as we can tx only 7 bytes of data in segmented tx.
                }
                else
                {
                  number_of_segments = (od_data_length / 7) + 1;
                }
                debugV("Segmnents: %i", number_of_segments);
                sdo_initiate_upload_response(od_index, od_sub_index, od_data_length, rframe);
              }
            }
          }
        }
      }
      break;

      case INITIATE_SEGMENT_UPLOAD_REQUEST:
        debugV("OD Len: %i ", od_data_length);
        debugV("Seg. No.: %i ", number_of_segments);
        debugV("Counter: %i: ", counter);

        if (counter != number_of_segments)
        {
          debugV("Toggle Bit: %x", sdo_toggle);

          if (((irq_frm.data[0] >> 4) & 0x01) == sdo_toggle) //check sdo toggle bit is correct or not, starting with 0;

          {
            sdo_upload_segmented_data(sdo_toggle, od_data_length, data_buffer, counter, rframe);
            sdo_toggle = !sdo_toggle;
            debugV("Toggle Bit: %x", sdo_toggle);
            counter++;
            debugV("Counter: %i", counter);
          }
          else
          {
            sdo_send_abort_code(od_index, od_sub_index, SDO_TOGGLE_BIT_NOT_ALTERED);
            debugV("Error Toggle Bit");
          }

        } //end if
        else
        {
          debugV("Number of Segments exceeded");
          delay(10000);
        }
        break;

      case NON_EXPEDITED_DWNLD_REQUEST:

        break;

      case REQUEST_SEGMENT_DOWNLOAD:

        break;

      case REQUEST_BLOCK_UPLOAD:

        break;

      case REQUEST_BLOCK_DOWNLOAD:

        break;

      case SDO_REQUEST_ABORT:

        break;
      }
    }
    break;

    case TPDO_3_MESSAGE:
    {
      can_frame rframe;

      rframe.id = TPDO_3_MESSAGE + CANOPEN_NODE_ID;
      rframe.extended = 0;
      rframe.dlc = 8;

      if (frame.data[0] == (0x80 | CANOPEN_NODE_ID) &&
          frame.data[4] == 0x1)
      {

        od_index = frame.data[1] | (frame.data[2] << 8);
        od_sub_index = frame.data[3];
        odindex = od_find_index(od_index);
        if (odindex != -1)
        {
          int len = od_find_data_length(odindex, od_sub_index);

          if (len > 0)
          {
            od_read_data(odindex, od_sub_index, data_buffer, len);

            if (len <= 4)
            {
              debugV("TPDO Expedited upload for 0x%X:%d", od_index, od_sub_index);
              rframe.data[0] = CANOPEN_NODE_ID;
              rframe.data[1] = frame.data[1];
              rframe.data[2] = frame.data[2];
              rframe.data[3] = frame.data[3];
              rframe.data[4] = 0x0;
              rframe.data[5] = 0x0;
              rframe.data[6] = 0x0;
              rframe.data[7] = 0x1;
              for (int i = 0; i < len; i++)
              {
                rframe.data[i + 4] = data_buffer[i];
              }
              can_send_frame(rframe);
            }
          }
        }
      }
    }
    break;

    case 0x700:
    {
      byte state = frame.data[0];
      debugV("Hearbeat received");
      if (state == 0x00)
        canopen_state = INITIALIZATION;
      else if (state == 0x04)
        canopen_state = STOPPED;
      else if (state == 0x05)
        canopen_state = OPERATIONAL;
      else if (state == 0x80)
        canopen_state = PRE_OPERATIONAL;
    }
    break;
    }
  }

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

  if (lastCmd == "gettime")
  {
    getTimefromNTP();

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
