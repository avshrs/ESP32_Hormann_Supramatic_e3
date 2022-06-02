#include <stdint.h>
#include <stdbool.h>
#include "hoermann.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <unistd.h>
#include <algorithm> // std::fill

#include <ctime> // localtime
#include <stdlib.h>
#include <sstream> // stringstream

void Hoermann::init(int tx_pin)
{

    ser.serial_open(tx_pin);
}

void Hoermann::run_loop(void)
{
    Serial2.setTimeout(3);
    // Serial2.setRxBufferSize(8);

    unsigned long start = millis();


    for (int i = 0; i < 7; i++)
    {
        rx_buf.buf[i] = 0x00;
        tx_buf.buf[i] = 0x00;
    }
    tx_buf.size = 0;
    rx_buf.size = 0;


    ser.serial_read(rx_buf);
    int s = millis();
    start = millis();
     print_buffer(rx_buf.buf,rx_buf.size);
    if (is_frame_corect(rx_buf))
    {
        if (is_slave_query(rx_buf))
        { 
            print_buffer(rx_buf.buf,rx_buf.size);
            if (is_slave_scan(rx_buf))
            {
                make_scan_responce_msg(rx_buf, tx_buf);
                while (true)
                {
                    if ((millis() - start) > (0))
                    {
                        if ((millis() - start) > 7)
                        {
                            break;
                        }

                        ser.serial_send(tx_buf);
                        break;
                    }
                }
            }
            else if (is_slave_status_req(rx_buf))
            {
               
                make_status_req_msg(rx_buf, tx_buf);

                while (true)
                {

                    if ((millis() - start) > (0))
                    {
                        if ((millis() - start) > 7)
                        {
                            break;
                        }
                        print_buffer(tx_buf);
                        ser.serial_send(tx_buf);
                        break;
                    }

                }
            }
        }
        else if (is_broadcast(rx_buf))
        {
            if (is_broadcast_lengh_correct(rx_buf))
            {
                update_broadcast_status(rx_buf);
            }
        }
        
    }
}

void Hoermann::update_broadcast_status(RX_Buffer &buf)
{
    if (static_cast<uint8_t>(broadcast_status) != buf.buf[2])
    {
        broadcast_status = static_cast<uint16_t>(buf.buf[2]);
    }
}

uint8_t Hoermann::get_length(RX_Buffer &buf)
{
    if (buf.size > 2)
    {
        return buf.buf[1] & 0x0F;
    }
    else
        return 0x00;
}

uint8_t Hoermann::get_counter(RX_Buffer &buf)
{
    if (buf.size > 2)
    {
        return (buf.buf[1] & 0xF0) + 0x10;
    }
    else
        return 0x00;
}

bool Hoermann::is_broadcast(RX_Buffer &buf)
{
    if (buf.size == 5)
    {
        if (buf.buf[0] == BROADCAST_ADDR && calc_crc8(buf.buf, 4) == buf.buf[4])
        {
            // print_buffer(buf.buf.data(), buf.size);
            return true;
        }
        else
            return false;
    }
    else
    {
        return false;
    }
}

bool Hoermann::is_slave_query(RX_Buffer &buf)
{
    if (buf.size > 3 && buf.size < 6)
    {
        if (buf.buf[0] == UAP1_ADDR)
            return true;
        else
            return false;
    }
    else
    {
        return false;
    }
}
bool Hoermann::is_frame_corect(RX_Buffer &buf)
{
    if (buf.size > 3 && buf.size < 6)
    {
        if (calc_crc8(buf.buf, buf.size - 1) == buf.buf[buf.size - 1])
            return true;
        else
            return false;
    }
    else
    {
        return false;
    }
}

bool Hoermann::is_slave_scan(RX_Buffer &buf)
{
    if (buf.size == 5)
    {
        if (is_broadcast_lengh_correct(buf) && (buf.buf[2] == CMD_SLAVE_SCAN))
            return true;
        else
            return false;
    }
    else
    {
        return false;
    }
}

bool Hoermann::is_slave_status_req(RX_Buffer &buf)
{
    if (buf.size == 4)
    {
        if (is_req_lengh_correct(buf) && (buf.buf[2] == CMD_SLAVE_STATUS_REQUEST))
            return true;
        else
            return false;
    }
    else
    {
        return false;
    }
}

bool Hoermann::is_broadcast_lengh_correct(RX_Buffer &buf)
{
    if (get_length(buf) == broadcast_lengh)
        return true;
    else
        return false;
}

bool Hoermann::is_req_lengh_correct(RX_Buffer &buf)
{
    if (get_length(buf) == reguest_lengh)
        return true;
    else
        return false;
}

void Hoermann::print_buffer(uint8_t *buf, int size )
{

    if (size > 0)
    {
        Serial.print((int)size, DEC);
        Serial.print(" | ");
        for (int i = 0; i < (int)size; i++)
        {

            Serial.print(" ");
            Serial.print(buf[i], HEX);
        }
        Serial.println();
    }
}

void Hoermann::print_buffer(TX_Buffer &buf)
{

    if (buf.size > 0)
    {
        Serial.print((int)buf.size, DEC);
        Serial.print(" | ");
        for (int i = 0; i < (int)buf.size; i++)
        {

            Serial.print(" ");
            Serial.print(buf.buf[i], HEX);
        }
        Serial.println();
    }
}

uint8_t Hoermann::get_master_address()
{
    return master_address;
}

void Hoermann::make_scan_responce_msg(RX_Buffer &rx_buf, TX_Buffer &tx_buf)
{
    tx_buf.buf[0] = get_master_address();
    tx_buf.buf[1] = 0x02 | get_counter(rx_buf);
    tx_buf.buf[2] = UAP1_TYPE;
    tx_buf.buf[3] = UAP1_ADDR;
    tx_buf.buf[4] = calc_crc8(tx_buf.buf, 4);
    tx_buf.timeout = 1;
    tx_buf.size = 5;
}

void Hoermann::make_status_req_msg(RX_Buffer &rx_buf, TX_Buffer &tx_buf)
{
    tx_buf.buf[0] = get_master_address();
    tx_buf.buf[1] = 0x03 | get_counter(rx_buf);

    tx_buf.buf[2] = CMD_SLAVE_STATUS_RESPONSE;
    if (slave_respone_data == RESPONSE_STOP)
    {
        tx_buf.buf[3] = 0x00;
        tx_buf.buf[4] = 0x00;
    }
    else
    {
        tx_buf.buf[3] = static_cast<uint8_t>(slave_respone_data);
        tx_buf.buf[4] = 0x10;
    }
    slave_respone_data = RESPONSE_DEFAULT;
    tx_buf.buf[5] = calc_crc8(tx_buf.buf, 5);
    tx_buf.timeout = 1;
    tx_buf.size = 6;
}

String Hoermann::get_state()
{
    if ((broadcast_status) == 0x00)
    {
        String stat = "venting";
        return stat;
    }
    else if ((broadcast_status) == 0x02)
    {
        String stat = "closed";
        return stat;
    }
    else if ((broadcast_status) == 0x01)
    {
        String stat = "open";
        return stat;
    }
    else
    {
        String stat = "error";
        return stat;
    }
}
// String Hoermann::get_state()
// {
//   if ((broadcast_status & 0x01) == 0x01)
//   {
//     return cfg->get_stopped_string();
//   }
//   else if ((broadcast_status & 0x02) == 0x02)
//   {
//     return cfg->get_open_string();
//   }
//   else if ((broadcast_status & 0x80) == 0x80)
//   {
//     return cfg->get_closed_string();
//   }
//   else if ((broadcast_status & 0x60) == 0x40)
//   {
//     return  cfg->get_venting_string();
//   }
//   else if ((broadcast_status & 0x60) == 0x60)
//   {
//     return  cfg->get_opening_string();
//   }
//   else if ((broadcast_status & 0x10) == 0x10)
//   {
//     return cfg->get_closing_string();
//   }
//   else if (broadcast_status == 0x00)
//   {
//     return cfg->get_error_string();
//   }
//   else
//     return cfg->get_offline_string();

// }

void Hoermann::set_state(String action)
{
    if (action == "stop")
    {
        slave_respone_data = RESPONSE_STOP;
    }
    else if (action == "open")
    {
        slave_respone_data = RESPONSE_OPEN;
    }
    else if (action == "close")
    {
        slave_respone_data = RESPONSE_CLOSE;
    }
    else if (action == "venting")
    {
        slave_respone_data = RESPONSE_VENTING;
    }
    else if (action == "light")
    {
        slave_respone_data = RESPONSE_TOGGLE_LIGHT;
    }
}

uint8_t Hoermann::calc_crc8(uint8_t *p_data, uint8_t len)
{
    size_t i;
    uint8_t crc = CRC8_INITIAL_VALUE;
    while (len--)
    {
        crc ^= *p_data++;
        for (i = 0; i < 8; i++)
        {
            if (crc & 0x80)
            {
                crc <<= 1;
                crc ^= 0x07;
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return (crc);
}

void Hoermann::door_open()
{
    set_state("open");
}

void Hoermann::door_close()
{
    set_state("close");
}

void Hoermann::door_venting()
{
    set_state("venting");
}

void Hoermann::door_toggle_light()
{
    set_state("light");
}

void Hoermann::door_stop()
{
    set_state("stop");
}
