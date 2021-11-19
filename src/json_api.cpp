#include "json_api.h"
#include <map>

std::map<std::string, int> outlet_modes{
    {"Hand/AUS", 0},
    {"Auto", 2},
    {"Hand/EIN", 4},
    {"Hand/AUF", 5},
    {"Hand/ZU", 6},
};
std::map<std::string, int> operating_modes{
    {"Zeit/Auto", 0},
    {"Normal", 1},
    {"Abgesenkt", 2},
    {"Standby/Frostschutz", 3},
};

std::map<int, std::string> operating_modes_to_str{
    {0, "Zeit/Auto"},
    {1, "Normal"},
    {2, "Abgesenkt"},
    {3, "Standby/Frostschutz"},
};

namespace JsonAPI
{

    String sendJsonData()
    {

        connect(CANOPEN_NODE_ID);

        int err = 0;

            byte data_buffer[CANOPEN_MAX_DATA_BUFFER_LENGTH];
            byte data_length;

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

            Setting _settings[] = {

                NewSettings(0x01)

            };

            Variable _variables = NewVariables(0x01);

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
            JsonArray data_settings = data.createNestedArray("Settings");
            JsonArray data_variables = data.createNestedArray("Variables");

            while (ptr_outlet < endPtr_outlet)
            {
                std::string descr = "", mode = "";
                int state = 0;
                int imode = 0;
                int unit = 0;

                readOutlet(ptr_outlet, &descr, &imode, &mode, &state, &unit);

                if (descr != "")
                {

                    if (mode == "")
                    {
                        mode = "na";
                    }
                    JsonObject data_outputs_output = data_outputs.createNestedObject();

                    data_outputs_output["Number"] = ptr_outlet->Description.SubIndex;
                    data_outputs_output["Description"] = descr;
                    data_outputs_output["Index"] = ptr_outlet->Mode.idx.to_uint16();
                    data_outputs_output["Subindex"] = ptr_outlet->Mode.SubIndex;

                    JsonObject data_outputs_output_value = data_outputs_output.createNestedObject("Value");

                    data_outputs_output_value["Value"] = state;
                    data_outputs_output_value["Mode"] = mode;
                    data_outputs_output_value["Unit"] = unit;
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

            // yield();

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

            JsonObject data_settings_setting = data_settings.createNestedObject();

            getObject(_settings->OperationMode, data_buffer, &data_length);

            data_settings_setting["Description"] = "OperatingMode";
            data_settings_setting["Index"] = _settings->OperationMode.idx.to_uint16();
            data_settings_setting["Subindex"] = _settings->OperationMode.SubIndex;
            JsonObject data_settings_setting_value = data_settings_setting.createNestedObject("Value");
            data_settings_setting_value["Value"] = readStringfromIndex(data_buffer, &data_length);
            data_settings_setting_value["Unit"] = 48;

            JsonArray data_settings_setting_OperatingMode_options = data_settings_setting_value.createNestedArray("Options");
            data_settings_setting_OperatingMode_options.add("Zeit/Auto");
            data_settings_setting_OperatingMode_options.add("Normal");
            data_settings_setting_OperatingMode_options.add("Abgesenkt");
            data_settings_setting_OperatingMode_options.add("Standby/Frostschutz");

            data_settings_setting = data_settings.createNestedObject();

            getObject(_settings->WaterPriority, data_buffer, &data_length);
            
                data_settings_setting["Description"] = "WaterPriority";
                data_settings_setting["Index"] = _settings->WaterPriority.idx.to_uint16();
                data_settings_setting["Subindex"] = _settings->WaterPriority.SubIndex;
                data_settings_setting_value = data_settings_setting.createNestedObject("Value");
                data_settings_setting_value["Value"] = readStringfromIndex(data_buffer, &data_length);
                JsonArray data_settings_setting_WaterPriority_options = data_settings_setting_value.createNestedArray("Options");

                data_settings_setting_WaterPriority_options.add("EIN");
                data_settings_setting_WaterPriority_options.add("AUS");
  

            data_settings_setting = data_settings.createNestedObject();

            getObject(_settings->WaterTargetTemperature, data_buffer, &data_length);
            
                data_settings_setting["Description"] = "WaterTargetTemperature";
                data_settings_setting["Index"] = _settings->WaterTargetTemperature.idx.to_uint16();
                data_settings_setting["Subindex"] = _settings->WaterTargetTemperature.SubIndex;
                data_settings_setting_value = data_settings_setting.createNestedObject("Value");

                data_settings_setting_value["Value"] = readfloatfromIndex(data_buffer, &data_length);
                data_settings_setting_value["Unit"] = 1;
 

            JsonObject data_variables_variable = data_variables.createNestedObject();

            getObject(_variables.FlowTargetTemperature, data_buffer, &data_length);
            
                data_variables_variable["Description"] = "FlowTargetTemperature";
                data_variables_variable["Value"] = readfloatfromIndex(data_buffer, &data_length);
                data_variables_variable["Unit"] = 1;


            data_variables_variable = data_variables.createNestedObject();
            getObject(_variables.Operation, data_buffer, &data_length);
            
                data_variables_variable["Description"] = "Operation";

                std::map<int, std::string>::iterator it = operating_modes_to_str.find(readintfromIndex(data_buffer, &data_length));

                if (it != operating_modes_to_str.end())
                {
                    data_variables_variable["Value"] = it->second;
                }
                else
                {
                    data_variables_variable["Value"] = 999;
                }
                data_variables_variable["Unit"] = 48;


            String json_data;


                serializeJsonPretty(jsonBuffer, json_data);

                return json_data;

        

    }

    int readJsonData(String json_data)
    {

        DynamicJsonDocument doc(400);

        DeserializationError error = deserializeJson(doc, json_data);

        if (error)
        {
            debugV("deserializeJson() failed: ");
            debugV("%s", error.c_str());
            return 500;
        }

        else
        {

            char unit = doc["Value"]["Unit"].as<char>();

            char data_buffer[7];
            byte len;

            if (unit == 1)
            {

                int32_t state = doc["Value"]["newValue"].as<int32_t>();
                std::copy(static_cast<const char *>(static_cast<const void *>(&state)),
                          static_cast<const char *>(static_cast<const void *>(&state)) + sizeof(state),
                          data_buffer);

                len = sizeof(int32_t);
            }

            if (unit == 43)
            {

                std::string mode = doc["Value"]["Mode"].as<std::string>();
                data_buffer[0] = outlet_modes.at(mode);
                len = 1;

                debugV("db[0]: %X", data_buffer[0]);
                debugV("db[1]: %X", data_buffer[1]);
            }

            if (unit == 48)
            {

                std::string mode = doc["Value"]["Mode"].as<std::string>();
                data_buffer[0] = unit;
                data_buffer[1] = operating_modes.at(mode);
                len = 2;

                debugV("db[0]: %X", data_buffer[0]);
                debugV("db[1]: %X", data_buffer[1]);
            }

            uint16_t index = doc["Index"].as<uint16_t>();
            uint8_t subindex = doc["Subindex"].as<uint8_t>();

            ObjectIndex oindex = NewObjectIndex(index, subindex);

            setState(oindex, data_buffer, len);

            return 200;
        }
    }

}