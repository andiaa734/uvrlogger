#include "Arduino.h"
#include <web.h>
#include "can.h"

#include "uvr16x2.h"

char nodeid;
uint32_t rescobid;
bool connected = false;
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>CAN Frame Builder</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Times New Roman; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem; color: #FF0000;}
  </style>
  </head><body>
  <h2>CAN Frame Builder</h2> 
  <form action="/get">
    COB ID: <input type="text" name="cobid">
  <br>
    Response COB ID: <input type="text" name="rescobid">
  <br>
    Node ID: <input type="text" name="nodeid">
  <br>
    DLC: <input type="text" name="dlc">
  <br>
    Data Byte 0: <input type="text" name="b0" value="0x0">
  <br>
    Data Byte 1: <input type="text" name="b1" value="0x0">
  <br>
    Data Byte 2: <input type="text" name="b2" value="0x0">
  <br>
    Data Byte 3: <input type="text" name="b3" value="0x0">
  <br>
    Data Byte 4: <input type="text" name="b4" value="0x0">
  <br>
    Data Byte 5: <input type="text" name="b5" value="0x0">
  <br>
    Data Byte 6: <input type="text" name="b6" value="0x0">
  <br>
    Data Byte 7: <input type="text" name="b7" value="0x0">
  <br>
    <input type="submit" value="Submit">
  <br>
    <form action="/disconnect">
    <input type="submit" value="Disconnect">

  </form>
</body></html>)rawliteral";

namespace Web
{

  ESP8266WebServer server(80);

  void start()
  {

    server.on("/", handleRoot);
    server.on("/disconnect", handleDisconnect);

    server.on("/get", handleForm); //form action is handled here

    server.on("/json", HTTP_GET, datatojson);
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

    for (int i = 0; i < 7; i++)
    {

      debugV("Send Data [%i]: %X", i, frame.data[i]);
    }

    can_send_frame(frame, rescobid);
    wait(rescobid, frame);

    server.send(200, "text/html", index_html); //Send web page
  }

  void datatojson()
  {

    Inlet _inlets[] = {
        NewInlet(0x0),
        NewInlet(0x1),
        NewInlet(0x2),
        NewInlet(0x3),
        NewInlet(0x4),
        NewInlet(0x5),
        NewInlet(0x6),
        NewInlet(0x7),
        NewInlet(0x8),
        NewInlet(0x9)
        /* NewInlet(0xa),
        NewInlet(0xb),
        NewInlet(0xc),
        NewInlet(0xd),
        NewInlet(0xe),
        NewInlet(0xf)*/
    };

    Outlet _outlets[] = {
        NewOutlet(0x0),
        NewOutlet(0x1),
        NewOutlet(0x2),
        //  NewOutlet(0x3),
        NewOutlet(0x4),
        NewOutlet(0x5)
        /* NewOutlet(0x6),
        NewOutlet(0x7),
        NewOutlet(0x8),
        NewOutlet(0x9),
        NewOutlet(0xa),
        NewOutlet(0xb),
        NewOutlet(0xc),
        NewOutlet(0xd),
        NewOutlet(0xe),
        NewOutlet(0xf) */
    };

    Heatmeter _heatmeters[] = {

        NewHeatmeter(0xc)

    };

    struct Inlet *ptr_inlet = _inlets;
    struct Inlet *endPtr_inlet = _inlets + sizeof(_inlets) / sizeof(_inlets[0]);

    struct Outlet *ptr_outlet = _outlets;
    struct Outlet *endPtr_outlet = _outlets + sizeof(_outlets) / sizeof(_outlets[0]);

    struct Heatmeter *ptr_heatmeter = _heatmeters;
    struct Heatmeter *endPtr_heatmeter = _heatmeters + sizeof(_heatmeters) / sizeof(_heatmeters[0]);

    DynamicJsonDocument jsonBuffer(5000);

    JsonObject data = jsonBuffer.createNestedObject("Data");

    JsonArray data_inputs = data.createNestedArray("Inputs");
    JsonArray data_outputs = data.createNestedArray("Outputs");
    JsonArray data_heatmeters = data.createNestedArray("Heatmeters");

    connect(CANOPEN_NODE_ID);

    while (ptr_outlet < endPtr_outlet)
    {
      std::string descr = "", mode = "";
      int state = 0;
      int imode = 0;

      readOutlet(ptr_outlet, &descr, &imode, &mode, &state);

      if (descr != "")
      {

        if (mode == "")
        {
          mode = "na";
        }
        JsonObject data_outputs_output = data_outputs.createNestedObject();

        data_outputs_output["Number"] = ptr_outlet->Description.SubIndex;
        data_outputs_output["Description"] = descr;

        JsonObject data_outputs_output_value = data_outputs_output.createNestedObject("Value");

        data_outputs_output_value["Value"] = state;
        data_outputs_output_value["Mode"] = mode;
      }
      ptr_outlet++;
    }

    while (ptr_inlet < endPtr_inlet)
    {
      std::string descr = "", mode = "";
      int state = 0;
      float val = 0;
      int unit = 0;

      readInlet(ptr_inlet, &descr, &state, &val, &unit);

      if (descr != "")
      {

        JsonObject data_inputs_input = data_inputs.createNestedObject();

        data_inputs_input["Number"] = ptr_inlet->Description.SubIndex;
        data_inputs_input["Description"] = descr;
        data_inputs_input["AD"] = "A";

        JsonObject data_inputs_input_value = data_inputs_input.createNestedObject("Value");

        data_inputs_input_value["Value"] = val;
        data_inputs_input_value["Unit"] = Units[unit];
      }

      ptr_inlet++;
    }

    yield();

    while (ptr_heatmeter < endPtr_heatmeter)
    {
      std::string descr = "", state = "";
      int tfunit = 0, trunit = 0, flow, funit, upower, ptunit;
      float tflow, treturn, power, powertotal;

      readHeatmeter(ptr_heatmeter, &descr, &tflow, &tfunit, &treturn,
                    &trunit, &flow, &funit, &state, &power, &upower, &powertotal, &ptunit);

      if (descr != "")
      {

        JsonObject data_heatmeters_heatmeter = data_heatmeters.createNestedObject();

        data_heatmeters_heatmeter["Number"] = ptr_heatmeter->Description.SubIndex;
        data_heatmeters_heatmeter["Description"] = descr;
        data_heatmeters_heatmeter["State"] = state;

        JsonObject data_heatmeters_heatmeter_value = data_heatmeters_heatmeter.createNestedObject("Value");

        data_heatmeters_heatmeter_value["Flow"] = flow;
        data_heatmeters_heatmeter_value["FlowUnit"] = Units[funit];
        data_heatmeters_heatmeter_value["Flowtemperature"] = tflow;
        data_heatmeters_heatmeter_value["FlowtempUnit"] = Units[tfunit];
        data_heatmeters_heatmeter_value["Returntemperature"] = treturn;
        data_heatmeters_heatmeter_value["ReturntempUnit"] = Units[trunit];
        data_heatmeters_heatmeter_value["Power"] = power;
        data_heatmeters_heatmeter_value["PowerUnit"] = Units[upower];
        data_heatmeters_heatmeter_value["PowerTotal"] = powertotal;
        data_heatmeters_heatmeter_value["PowerTotalUnit"] = Units[ptunit];
      }

      ptr_heatmeter++;
    }


    String json_data;
    serializeJsonPretty(jsonBuffer, json_data);
    server.send(200, "application/json;charset=utf-8", json_data);
  }

  void readJSON()
  {

    DynamicJsonDocument doc(200);

    if (server.args() == 0)
      return; // we could do in the caller an error handling on that

    DeserializationError error = deserializeJson(doc, server.arg("plain"));

    if (error)
    {
      debugV("deserializeJson() failed: ");
      debugV("%s", error.c_str());
        server.send(500);

      return;
    }

    // Extract values
    debugV("Response:");
    debugV("Description: %s", doc["Description"].as<const char *>());
    debugV("Value: %i", doc["Value"]["Value"].as<int>());
    debugV("Mode: %s", doc["Value"]["Mode"].as<const char *>());
        server.send(200);

    Outlet outlet = NewOutlet(doc["Number"].as<uint8_t>());
    int newState = doc["Value"]["imode"].as<int>();

    setState(outlet, newState);

    
      
  }
}