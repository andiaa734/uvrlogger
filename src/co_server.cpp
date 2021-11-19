#include "Arduino.h"
#include "co_server.h"
#include "canopen.h"
#include "sdo.h"
#include "od.h"

char data_buffer[CANOPEN_MAX_DATA_BUFFER_LENGTH];
unsigned char od_sub_index;
int od_index, odindex;
char number_of_segments, counter = 0;
char od_data_length;
char sdo_message_type, state_changed = 1, sdo_toggle;
char canopen_state = INITIALIZATION;

void handleCOServer(can_frame &frame)
{

    uint16_t cob_id = frame.id & 0x780;

    switch (cob_id)
    {

    case 0x0:

        //Broadcast Message

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
        //rframe.data[0] = INITIATE_SDO_UPLOAD_REQUEST | TRANSFER_SIZE_INDICATED;

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
                if (od_data_length > 0)
                {
                    od_read_data(odindex, od_sub_index, data_buffer, od_data_length);

                    if (od_data_length <= 4)
                    {


                        sdo_upload_expedited_data(od_index, od_sub_index, od_data_length, data_buffer);
                        /* rframe.data[0] |= EXPEDITED_UPLOAD;
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
              can_send_frame(rframe);*/
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
                            sdo_initiate_upload_response(od_index, od_sub_index, od_data_length, rframe);
                        }
                    }
                }
            }
        }
        break;

        case INITIATE_SEGMENT_UPLOAD_REQUEST:
/*             debugV("OD Len: %i ", od_data_length);
            debugV("Seg. No.: %i ", number_of_segments);
            debugV("Counter: %i: ", counter); */

            if (counter != number_of_segments)
            {
                debugV("Toggle Bit: %x", sdo_toggle);

                if (((irq_frm.data[0] >> 4) & 0x01) == sdo_toggle) //check sdo toggle bit is correct or not, starting with 0;

                {
                    sdo_upload_segmented_data(sdo_toggle, od_data_length, data_buffer, counter, rframe);
                    sdo_toggle = !sdo_toggle;
                    counter++;
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

    case NMT_HEARTBEAT_MESSAGE:
    {
        byte state = frame.data[0];
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

