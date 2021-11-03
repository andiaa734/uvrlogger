
#include "esphome2can.h"
#include <WiFiClientSecure.h>
#include "od.h"
#include "can.h"
#include "canopen.h"

WiFiClientSecure client;

const char *host = "homecontrol.suether.net";
const char *url = "https://homecontrol.fritz.box:8443/api/states/climate.heizung";
const uint16_t port = 8443;

void setupHTTPClient()
{
    WiFiClientSecure client;
}



DynamicJsonDocument getRoomTemperature()
{
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    bool mfln = client->probeMaxFragmentLength(host, 8443, 1024);
    if (mfln)
        client->setBufferSizes(1024, 1024);
    client->setInsecure();

    HTTPClient https;

    if (https.begin(*client, url))
    {

        https.addHeader("Content-Type", "application/json");

        https.addHeader("Authorization", bearer_token);

        int httpCode = https.GET();

        DynamicJsonDocument doc(800);
        deserializeJson(doc, https.getStream());
        return doc;
    }

    else
    {
        DynamicJsonDocument doc(1);

        return doc;
    }
}

void getTemperatureToOD()
{

    DynamicJsonDocument data = getRoomTemperature();

    if (data.size() > 0)
    {
        float currentTemp = data["attributes"]["current_temperature"].as<float>();
        debugV("Current Temp: %f", currentTemp);
        int16_t cTemp = currentTemp * 10;

        char od_sub_index = 0x1;
        int odindex = od_find_index(0x4E01);

        char data_buf[2];

        std::copy(static_cast<const char *>(static_cast<const void *>(&cTemp)),
                  static_cast<const char *>(static_cast<const void *>(&cTemp)) + sizeof cTemp,
                  data_buf);

        od_write_data(odindex, od_sub_index, data_buf, 2);

        can_frame frame;
        frame.id = RPDO_0_MESSAGE;
        frame.dlc = 8;
        frame.remote = 0;
        frame.extended = 0;
        frame.data[0] = data_buf[0];
        frame.data[1] = data_buf[1];
        frame.data[2] = 0x0;
        frame.data[3] = 0x0;
        frame.data[4] = 0x0;
        frame.data[5] = 0x0;
        frame.data[6] = 0x0;
        frame.data[7] = 0x0;

        can_send_frame(frame);
        



    }
    else
    {
        debugV("No Data received");
    }
}