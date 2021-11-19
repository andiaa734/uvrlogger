#include "uvr16x2.h"
#include "sdo.h"
#include "web.h"
#include <iostream>
#include <codecvt>
#include <string>
#include <locale>

ObjectIndex NewObjectIndex(uint16 index, uint8 subIndex)
{

    ObjectIndex newOI;

    newOI.idx.b0 = byte(index & 0xFF);
    newOI.idx.b1 = byte(index >> 8);
    newOI.SubIndex = subIndex;

    return newOI;
}

Setting NewSettings(uint8 subIndex)
{

    Setting newSetting;

    newSetting.OperationMode = NewObjectIndex(0x283B, 0x01);
    newSetting.WaterPriority = NewObjectIndex(0x2414, 0x00);
    newSetting.WaterTargetTemperature = NewObjectIndex(0x2414, 0x03);

    return newSetting;
}

Variable NewVariables(uint8 subIndex)
{

    Variable newVariable;

    newVariable.FlowTargetTemperature = NewObjectIndex(0x2D01, 0x01);
    newVariable.Operation = NewObjectIndex(0x2D0F, 0x01);

    return newVariable;
}

Inlet NewInlet(uint8 subIndex)
{

    Inlet newInlet;

    newInlet.Description = NewObjectIndex(0x200f, subIndex);
    newInlet.Value = NewObjectIndex(0x2050, subIndex);
    newInlet.Mode = NewObjectIndex(0x2051, subIndex);
    newInlet.State = NewObjectIndex(0x2052, subIndex);

    return newInlet;
}

Outlet NewOutlet(uint8 subIndex)
{

    Outlet newOutlet;
    newOutlet.Description = NewObjectIndex(0x208F, subIndex);
    newOutlet.StartDelay = NewObjectIndex(0x2094, subIndex);
    newOutlet.RunOnTime = NewObjectIndex(0x20a4, subIndex);
    newOutlet.Mode = NewObjectIndex(0x2091, subIndex);
    newOutlet.State = NewObjectIndex(0x20D0, subIndex);
    newOutlet.SpeedStage = NewObjectIndex(0x20D1, subIndex);
    return newOutlet;
}

Heatmeter NewHeatmeter(uint8 subIndex)
{

    Heatmeter newHeatmeter;
    newHeatmeter.Description = NewObjectIndex(0x280F, subIndex); // sub 12
    newHeatmeter.Flowtemperatur = NewObjectIndex(0x2B0B, subIndex);
    newHeatmeter.Returntemperatur = NewObjectIndex(0x2B11, subIndex);
    newHeatmeter.Flow = NewObjectIndex(0x2B17, subIndex);
    newHeatmeter.State = NewObjectIndex(0x2812, subIndex);
    newHeatmeter.Power = NewObjectIndex(0x2D01, subIndex);
    newHeatmeter.PowerTotal = NewObjectIndex(0x2D17, subIndex);

    return newHeatmeter;
}

void connect(uint8_t clientID)
{

    can_frame frm;
    can_frame rframe;
    frm.id = uint16(RPDO_2_MESSAGE);
    frm.dlc = 8;
    frm.extended = 0;
    frm.data[0] = 0x80 + byte(0x01);
    frm.data[1] = 0x00;
    frm.data[2] = 0x1F;
    frm.data[3] = 0x00;
    frm.data[4] = byte(0x01);
    frm.data[5] = byte(clientID);
    frm.data[6] = 0x80;
    frm.data[7] = 0x12;

    can_send_frame(frm, 0x401);

    wait(0x401, rframe);
    

        if (rframe.data[0] != 0x80 + byte(CANOPEN_NODE_ID))
        {
            debugV("Invalid MPDO address %x", rframe.data[0]);
        }

        else if (rframe.data[4] != 0x40 + byte(CANOPEN_NODE_ID) || rframe.data[5] != 0x06)
        {
            debugV("Invalid 0x640 + client id %X %X", rframe.data[5], rframe.data[4]);
        }

        else if (rframe.data[7] != 0x00)
        {
            debugV("Invalid byte 7 %X", rframe.data[7]);
        }

}

void disconnect(uint8_t clientID)
{
    can_frame frm;

    frm.id = uint16(RPDO_2_MESSAGE);
    frm.dlc = 8;
    frm.data[0] = 0x80 + byte(0x01);
    frm.data[1] = 0x01;
    frm.data[2] = 0x1F;
    frm.data[3] = 0x00;
    frm.data[4] = byte(0x01);
    frm.data[5] = byte(clientID);
    frm.data[6] = 0x80;
    frm.data[7] = 0x12;

    can_send_frame(frm);
}

/* void readHeatMeters()
{

    Heatmeter heatmeters[] = {
                 NewHeatmeter(0x0),
                NewHeatmeter(0x1),
                NewHeatmeter(0x2),
                NewHeatmeter(0x3),
                NewHeatmeter(0x4),
                NewHeatmeter(0x5),
                NewHeatmeter(0x11),
                NewHeatmeter(0x12),
                NewHeatmeter(0x13),
                NewHeatmeter(0x9),
                NewHeatmeter(0xa),
                NewHeatmeter(0xb), 
        NewHeatmeter(0xc),
                NewHeatmeter(0xd),
               NewHeatmeter(0xe),
               NewHeatmeter(0xf), 
    };

    struct Heatmeter *ptr = heatmeters;
    struct Heatmeter *endPtr = heatmeters + sizeof(heatmeters) / sizeof(heatmeters[0]);
    while (ptr < endPtr)
    {
        std::string descr = "", state = "";
        int tfunit = 0, trunit = 0, flow, funit, upower, ptunit;
        float tflow, treturn, power, powertotal;

        readHeatmeter(ptr, &descr, &tflow, &tfunit, &treturn, &trunit, &flow, &funit, &state, &power, &upower, &powertotal, &ptunit);

        if (descr == "")
        {

            descr = "Unbenutzt";
        }
        debugV("Wärmemenge %s", descr.c_str());
        debugV("Vorlauftemperatur: %.1f, %s", tflow, Units[tfunit]);
        debugV("Rücklauftemperatur: %.1f, %s", treturn, Units[trunit]);
        debugV("Durchfluss: %i, %s", flow, Units[funit]);
        debugV("Leistung: %.1f %s", power, Units[upower]);
        debugV("Leistung Gesamt: %.1f %s", powertotal, Units[ptunit]);
        debugV("Status: %s", state.c_str());

        ptr++;
    }
} */

void readHeatmeter(Heatmeter *heatmeter, std::string *descr, float *tflow, int *tfunit, float *treturn, int *trunit, int *flow, int *funit, std::string *state, float *power, int *upower, float *powertotal, int *ptunit)
{

    byte length;
    byte data_buffer[CANOPEN_MAX_DATA_BUFFER_LENGTH];
    int err = 1;

    getObject(heatmeter->Description, data_buffer, &length);

    *descr = readStringfromIndex(data_buffer, &length);

    getObject(heatmeter->Flowtemperatur, data_buffer, &length);

    *tflow = readfloatfromIndex(data_buffer, &length);
    *tfunit = (int)data_buffer[1];

    getObject(heatmeter->Returntemperatur, data_buffer, &length);

    *treturn = readfloatfromIndex(data_buffer, &length);
    *trunit = (int)data_buffer[1];

    getObject(heatmeter->Flow, data_buffer, &length);

    *flow = readfloatfromIndex(data_buffer, &length);
    *funit = (int)data_buffer[1];

    getObject(heatmeter->Power, data_buffer, &length);

    *power = readfloatfromIndex(data_buffer, &length);
    *upower = (int)data_buffer[1];

    getObject(heatmeter->PowerTotal, data_buffer, &length);

    *powertotal = readfloatfromIndex(data_buffer, &length);
    *ptunit = (int)data_buffer[1];

    getObject(heatmeter->State, data_buffer, &length);

    *state = readStringfromIndex(data_buffer, &length);

}

void readInlet(Inlet *inlet, std::string *descr, int *state, float *val, int *unit)
{
    byte length;
    byte data_buffer[CANOPEN_MAX_DATA_BUFFER_LENGTH];
    int err = 1;

    getObject(inlet->Description, data_buffer, &length);

    if (data_buffer[0] != 0x80)
    {
        *descr = readStringfromIndex(data_buffer, &length);
    }

    getObject(inlet->Value, data_buffer, &length);

    if (data_buffer[0] != 0x80)
    {

        *unit = (int)data_buffer[1];

        switch (data_buffer[1])
        {

        case 0x01: // °C
            *val = readfloatfromIndex(data_buffer, &length);
            break;

        case 0x03: // l/h
            *val = readintfromIndex(data_buffer, &length);
            break;

        default:
            break;
        }
    }

    getObject(inlet->State, data_buffer, &length);

    if (data_buffer[0] != 0x80)
    {
        *state = readintfromIndex(data_buffer, &length);
    }

}

void readOutlet(Outlet *outlet, std::string *descr, int *imode, std::string *mode, int *state, int *unit)
{

    byte length;
    byte data_buffer[CANOPEN_MAX_DATA_BUFFER_LENGTH];
    int err = 1;

    getObject(outlet->Description, data_buffer, &length);

    if (data_buffer[0] != 0x80)
    {
        *descr = readStringfromIndex(data_buffer, &length);
    }

    getObject(outlet->Mode, data_buffer, &length);

    if (data_buffer[0] != 0x80)
    {
        *mode = readStringfromIndex(data_buffer, &length);
    }

    getObject(outlet->State, data_buffer, &length);

    if (data_buffer[0] != 0x80)
        *state = readintfromIndex(data_buffer, &length);
    else
        *state = 0;

    *unit = (int)data_buffer[1];

}

void getObject(ObjectIndex index, byte *data_buffer, byte *len)
{

    char sdo_message_type;
    bool segmented_rx_last_frame = false;
    byte no_of_bytes, sdo_toggle, count;
    can_frame rframe;

    segmented_rx_last_frame = false;
    sdo_toggle = 0;
    count = 0;
    int err = 0;

    sdo_send_upload_request(index);
    wait((SSDOServerToClient2 + CANOPEN_NODE_ID), rframe);

    sdo_message_type = rframe.data[0];

    switch (sdo_message_type)
    {

    case EMERGENCY_MESSAGE:

        debugV("Error %x %x", rframe.data[7], rframe.data[6]);
        debugV("Databuffer: %x", data_buffer[0]);
        data_buffer[0] = 0x80;
        *len = 0;
        err = 0;
        break;

    case SDO_RESPONSE_READ_8BIT:
    case SDO_RESPONSE_READ_16BIT:
    case SDO_RESPONSE_READ_24BIT:
    case SDO_RESPONSE_READ_32BIT:

        no_of_bytes = 4 - ((rframe.data[0] >> 2) & 0x03); // number of bytes that do not have data in the can frame

        for (int i = 0; i < no_of_bytes; i++)
        {

            data_buffer[i] = rframe.data[i + 4];
        }

        *len = no_of_bytes;
        err = 1;
        break;

    case BEGIN_SDO_DOWNLOAD:

        sdo_download_segment_response(sdo_toggle);

        while (segmented_rx_last_frame == false) // check if this is last segment or not
        {
            wait(SSDOServerToClient2 + CANOPEN_NODE_ID, rframe);

            if (((rframe.data[0] >> 4) & 0x01) == sdo_toggle) // check for sdo toggle bit
            {
                // debugV("check for sdo toggle bit");

                if ((rframe.data[0] & 0x01) == 1) // check if last segment or not
                {
                    char temp_counter = 0;
                    segmented_rx_last_frame = true;
                    //  debugV("Last Segment received");

                    no_of_bytes = 7 - ((rframe.data[0] >> 1) & 0x07); // find how many bytes have valid data in the frame
                    while (temp_counter != no_of_bytes)
                    {
                        data_buffer[(int)count + temp_counter] = rframe.data[(int)temp_counter + 1];

                        temp_counter++;
                    }

                    *len = count + temp_counter;
                    err = 1;
                    break;
                }

                else
                {
                    data_buffer[0 + count] = rframe.data[1];
                    data_buffer[1 + count] = rframe.data[2];
                    data_buffer[2 + count] = rframe.data[3];
                    data_buffer[3 + count] = rframe.data[4];
                    data_buffer[4 + count] = rframe.data[5];
                    data_buffer[5 + count] = rframe.data[6];
                    data_buffer[6 + count] = rframe.data[7];
                    count += 7;
                }

                sdo_toggle = !sdo_toggle;

                sdo_download_segment_response(sdo_toggle);
            }
            else
            {
                // sdo_send_abort_code(inlet->Description.idx.b0, inlet->Description.SubIndex, SDO_TOGGLE_BIT_NOT_ALTERED);
                segmented_rx_last_frame = true;
                err = 0;
                debugV("SDO_TOGGLE_BIT_NOT_ALTERED");
            }
            // comm_timeout_time = millis();
        } // while

        break;

    default:
        debugV("Protokoll Error");
        err = 0;
        break;
    }
}

std::string readStringfromIndex(byte *input, byte *len)
{

    byte outputbuffer[*len];

    for (int i = 0; i < *len; i++)
    {
        // remove first 2 bytes --> just Datatype Identifier?
        outputbuffer[i] = input[i + 2];
    }

    std::u16string u16_str(reinterpret_cast<const char16_t *>(outputbuffer));

    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> cv;

    std::string str8 = cv.to_bytes(u16_str);

    return str8;
}

float readfloatfromIndex(byte *input, byte *len)
{

    int32_t value = 0;
    float result = 0.0;

    value |= input[2];
    value |= (((int32_t)input[3]) << 8);
    value |= (((int32_t)input[4]) << 16);
    value |= (((int32_t)input[5]) << 24);

    result = value / 10.0;

    return result;
}

int readintfromIndex(byte *input, byte *len)
{

    int32_t value = 0;

    value |= input[2];
    value |= (((int32_t)input[3]) << 8);
    value |= (((int32_t)input[4]) << 16);
    value |= (((int32_t)input[5]) << 24);

    return value;
}

/* void readInlets()
{

    Inlet inlets[] = {
        NewInlet(0x0),
        NewInlet(0x1),
        NewInlet(0x2),
        NewInlet(0x3),
        NewInlet(0x4),
        NewInlet(0x5),
       NewInlet(0x6),
        NewInlet(0x7),
        NewInlet(0x8),
        NewInlet(0x9),
        NewInlet(0xa),
        NewInlet(0xb),
        NewInlet(0xc),
        NewInlet(0xd),
        NewInlet(0xe),
        NewInlet(0xf),
    };

    debugV("+---------+-----------------+--------+------");
    debugV("| Eingang | Bezeichnung     | Status | Wert");
    debugV("+---------+-----------------+--------+------");

    struct Inlet *ptr = inlets;
    struct Inlet *endPtr = inlets + sizeof(inlets) / sizeof(inlets[0]);
    while (ptr < endPtr)
    {
        std::string descr = "Unbenutzt";
        int state = 0;
        float val = 0;
        int unit = 0;
        readInlet(ptr, &descr, &state, &val, &unit);

        if (descr == "")
        {
            descr = "Unbenutzt";
        }

        debugV("| %x | %s | %i | %.1f | %s", ptr->Description.SubIndex, descr.c_str(), state, val, Units[unit]);
        ptr++;
    }

    debugV("+---------+-----------------+--------+------");
} */

void readOutlets()
{

    Outlet outlets[] = {
        NewOutlet(0x0)
        /*         NewOutlet(0x6),
        NewOutlet(0x1),
        NewOutlet(0x2),
        //NewOutlet(0x3)
        NewOutlet(0x4),
        NewOutlet(0x5)
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

    struct Outlet *ptr = outlets;
    struct Outlet *endPtr = outlets + sizeof(outlets) / sizeof(outlets[0]);
    while (ptr < endPtr)
    {
        std::string descr = "", mode = "";
        int imode = 0;
        int state = 0;
        int unit = 0;

        readOutlet(ptr, &descr, &imode, &mode, &state, &unit);

        if (descr == "")
        {
            descr = "Unbenutzt";
        }
        debugV("| %X | %s | %i | %s | %i ", ptr->Description.SubIndex, descr.c_str(), imode, mode.c_str(), state);

        ptr++;
    }
}

void wait(uint32 Id, can_frame &rframe)
{

    unsigned long waiter_start = millis();

    while (millis() - waiter_start <= 4000)
    {

        if (millis() - waiter_start >= 4000)
        {
            debugV("TimeOut waiting for COB ID %X", Id);
            break;
        }

        if (irq_frm.id == Id)
        {

            rframe = irq_frm;
            irq_frm.id = 0;
            debugV("Response Frame ID: %X", rframe.id);

            for (int i = 0; i < rframe.dlc; i++)
            {

                debugV("frame[%i]: %X", i, rframe.data[i]);
            }
            break;
        }
        yield();
    }
}

void readBlock(ObjectIndex index, byte *data_buffer, byte *len)
{
    //byte data_buffer[CANOPEN_MAX_DATA_BUFFER_LENGTH];
    uint16_t server_crc;
    //int len = 0;
    char count = 0;
    can_frame rframe;

    sdo_send_block_upload_request(index);
    wait((SSDOServerToClient2 + CANOPEN_NODE_ID), rframe);

    debugV("Response received: %X", rframe.id);

    char res_command = rframe.data[0];
    char blksize = 0;

    ObjectIndex res_index;

    res_index.idx.b0 = rframe.data[1];
    res_index.idx.b1 = rframe.data[2];
    res_index.SubIndex = rframe.data[3];

    if ((res_command & 0xE0) != RESPONSE_BLOCK_UPLOAD)
    {
        debugV("No response for Block Download Request");
        return;
    }
    // Check that the message is for us

    if (res_index.idx.to_uint16() != index.idx.to_uint16() || res_index.SubIndex != index.SubIndex)
    {
        debugV("Wrong response received");
        return;
    }

    if (res_command & BLOCK_SIZE_SPECIFIED)
    {
        blksize = rframe.data[4];
        debugV("Size is %d bytes", blksize);
    }

    bool crc_supported = (res_command & CRC_SUPPORTED);

    sdo_initate_block_upload();

    char ackseq = 0;

    unsigned long end_time = millis();
    can_frame datafrm;

    while (millis() - end_time <= RESPONSE_TIMEOUT + 5000)
    {
        wait((SSDOServerToClient2 + CANOPEN_NODE_ID), datafrm);

        res_command = datafrm.data[0];

        char seqno = res_command & 0x7F;

        if (res_command & NO_MORE_BLOCKS)
        {
            char temp_counter = 0;
            int no_of_bytes = blksize - count; // find how many bytes have valid data in the frame
            while (temp_counter != no_of_bytes)
            {
                data_buffer[temp_counter + (seqno - 1) * 7] = datafrm.data[temp_counter + 1];

                temp_counter++;
            }

            debugV("Block received:");
            *len = temp_counter + count;

            for (int i = 0; i < *len; i++)
            {

                debugV("Data[%i] %X", i, data_buffer[i]);
            }

            debugV("ackseq: %X", ackseq);

            if (seqno == ackseq + 1)
            {
                ackseq = seqno;
            }

            sdo_send_ack_block(ackseq);
            wait((SSDOServerToClient2 + CANOPEN_NODE_ID), datafrm);

            if (*len == blksize)
            {
                ackseq = 0;
            }

            uint16_t crc = crc16.crcarc(data_buffer, 0, *len);
            res_command = datafrm.data[0];
            server_crc = datafrm.data[1];
            server_crc |= datafrm.data[2] << 8;
            debugV("CRC:");
            debugV("Server CRC: %X", server_crc);
            debugV("Client CRC: %X", crc);

            if (server_crc == crc)
            {
                sdo_end_block_upload();
                break;
            }
            else
            {
                debugV("CRC Error");
                break;
            }
        }

        else
        {

            data_buffer[0 + (seqno - 1) * 7] = datafrm.data[1];
            data_buffer[1 + (seqno - 1) * 7] = datafrm.data[2];
            data_buffer[2 + (seqno - 1) * 7] = datafrm.data[3];
            data_buffer[3 + (seqno - 1) * 7] = datafrm.data[4];
            data_buffer[4 + (seqno - 1) * 7] = datafrm.data[5];
            data_buffer[5 + (seqno - 1) * 7] = datafrm.data[6];
            data_buffer[6 + (seqno - 1) * 7] = datafrm.data[7];
            count += 7;
        }

        if (seqno == ackseq + 1)
        {
            ackseq = seqno;
        }
        else
        {
            sdo_send_ack_block(seqno);
        }
    } //while
}

void writeBlock(ObjectIndex index, byte *data_buffer, int len)
{

    int count = 0;
    can_frame frame;
    can_frame rframe;

    sdo_initate_block_download_request(index);
    wait((SSDOServerToClient2 + CANOPEN_NODE_ID), rframe);
    debugV("Response received: %X", rframe.id);
    Debug.handle();

    char res_command = rframe.data[0];
    char blksize;
    ObjectIndex res_index;

    res_index.idx.b0 = rframe.data[1];
    res_index.idx.b1 = rframe.data[2];
    res_index.SubIndex = rframe.data[3];

    if ((res_command & 0xE0) != RESPONSE_BLOCK_DOWNLOAD)
    {
        debugV("No response for Block Download Request");
        return;
    }
    // Check that the message is for us

    if (res_index.idx.to_uint16() != index.idx.to_uint16() || res_index.SubIndex != index.SubIndex)
    {
        debugV("Wrong response received");
        return;
    }

    char no_of_segments = 0, no_of_data_bytes = 0, counter = 0, segment_number = 0;
    unsigned long end_time = millis();

    if (len % 7 == 0)
    {
        no_of_segments = len / 7;
        debugV("no_of_seg : %i", no_of_segments);
    }
    else
    {
        no_of_segments = (len / 7) + 1;
        debugV("no_of_seg : %i", no_of_segments);
    }

    while (segment_number < no_of_segments && millis() - end_time <= RESPONSE_TIMEOUT + 5000)
    {

        if (segment_number == no_of_segments - 1)
        {
            frame.data[0] = REQUEST_BLOCK_DOWNLOAD | END_BLOCK_TRANSFER;

            if (len != 7)
            {
                if (len % 7 == 0)
                {
                    no_of_data_bytes = 7;

                    debugV("no_of_data_bytes: %i", no_of_data_bytes);
                }
                else
                {
                    no_of_data_bytes = (len % 7);
                    debugV("no_of_data_bytes: %i", no_of_data_bytes);
                }
            }
            else
            {
                no_of_data_bytes = 7;
                debugV("no_of_data_bytes: %i", no_of_data_bytes);
            }
            while (counter != 7)
            {
                if (counter < no_of_data_bytes)
                {
                    frame.data[counter + 1] = data_buffer[(segment_number * 7) + counter];
                    debugV("%X", frame.data[counter + 1]);
                }
                else
                {
                    frame.data[counter + 1] = 0;
                }
                counter++;
            }
        }
        else
        {
            frame.data[1] = data_buffer[segment_number * 7];
            frame.data[2] = data_buffer[(segment_number * 7) + 1];
            frame.data[3] = data_buffer[(segment_number * 7) + 2];
            frame.data[4] = data_buffer[(segment_number * 7) + 3];
            frame.data[5] = data_buffer[(segment_number * 7) + 4];
            frame.data[6] = data_buffer[(segment_number * 7) + 5];
            frame.data[7] = data_buffer[(segment_number * 7) + 6];
        }

        can_send_frame(frame);

        if (segment_number == no_of_segments - 1)
        {
            char ackseq = 0;

            wait((SSDOServerToClient2 + CANOPEN_NODE_ID), frame);
            debugV("Response received: %X", frame.id);
            Debug.handle();

            ackseq = frame.data[1];

            if (ackseq != segment_number)
            {
                segment_number = 0;
            }
            else
            {
                end_block_download(frame);
                break;
            }
        }

        segment_number++;

    } //while
}

void end_block_download(can_frame &frame)
{

    frame.data[0] = 0;
}

void setState(ObjectIndex index, char *data_buffer, int len)
{

    char data_length = 7;
    debugV("Index: %X", index.idx);

    connect(CANOPEN_NODE_ID);

    can_frame frame;
    can_frame rframe;

    frame.id = SSDOClientToServer2;
    frame.dlc = 8;
    frame.extended = 0;
    frame.remote = 0;

    uint16_t crc = crc16.crcarc((byte *)data_buffer, 0, len);

    data_buffer[len] = crc & 0xff;
    data_buffer[len + 1] = (crc >> 8);

    for (int i = len + 2; i < 7; i++)
    {
        data_buffer[i] = 0x00;
    }

    for (int i = 0; i < 7; i++)
    {

        debugV("Final Databuffer[%i]: %X", i, data_buffer[i]);
    }

    char number_of_segments = 0;
    char counter = 0;
    char sdo_toggle = 0;

    if (data_length % 7 == 0)
    {
        number_of_segments = (data_length / 7); //no. of segments = data length/7 as we can tx only 7 bytes of data in segmented tx.
    }
    else
    {
        number_of_segments = (data_length / 7) + 1;
    }

    sdo_initiate_download_request(index, data_length, frame);
    wait((SSDOServerToClient2 + CANOPEN_NODE_ID), rframe);

    char res_cmd = rframe.data[0];

    if (res_cmd != 0x60)
    {

        debugV("Unexpected Response Command %X", res_cmd);

        return;
    }

    unsigned long timeout = millis();

    while (counter != number_of_segments && millis() < timeout + 3000)
    {
        debugV("Toggle Bit: %x", sdo_toggle);

        if (((res_cmd >> 4) & 0x01) == sdo_toggle) //check sdo toggle bit is correct or not, starting with 0;

        {
            sdo_upload_segmented_data(sdo_toggle, data_length, data_buffer, counter, frame);
            wait((SSDOServerToClient2 + CANOPEN_NODE_ID), rframe);
            res_cmd = rframe.data[0];
            sdo_toggle = !sdo_toggle;
            debugV("Toggle Bit: %x", sdo_toggle);
            counter++;
            debugV("Counter: %i", counter);
        }
        else
        {
            sdo_send_abort_code(index.idx.to_uint16(), index.SubIndex, SDO_TOGGLE_BIT_NOT_ALTERED);
            debugV("Error Toggle Bit");
            break;
        }

    } //end if
}