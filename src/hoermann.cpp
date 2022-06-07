#include <stdint.h>
#include <stdbool.h>
#include "hoermann.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <unistd.h>
#include <algorithm> // std::fill
#include <sstream>
#include <ctime> // localtime
#include <stdlib.h>
#include <ostream> 

void Hoermann::init(int tx_pin)
{
    ser.serial_open(tx_pin);
}

void Hoermann::run_loop(void)
{
    RX_Buffer rx_buf;
    TX_Buffer tx_buf;
    unsigned long start;
    unsigned long s;
    ser.serial_read(rx_buf);

    s = micros();
    start = s;

    if (is_frame_corect(rx_buf))
    {   
        logy(buffer_to_string(rx_buf.buf,rx_buf.size), 5);
        if (is_slave_query(rx_buf))
        { 
            logy(buffer_to_string(rx_buf.buf,rx_buf.size), 3);
            if (is_slave_scan(rx_buf))
            {
                make_scan_responce_msg(rx_buf, tx_buf);
                while (true)
                {
                    if ((micros() - start) > (delay_msg))
                    {
                        if ((micros() - start) > max_frame_delay)
                        {
                            break;
                        }
                        scanning = true;

                        logy((String)("Pre Scan Response Time: "+(String)(micros() -s)), 4);

                        ser.serial_send(tx_buf);
                        scan_resp_time = micros() -s ;
                        logy(buffer_to_string(tx_buf.buf,tx_buf.size), 3);
                        logy((String)("Scan Response Time: "+ (String)(scan_resp_time)), 3);
                        break;
                    }
                }
            }
            else if (is_slave_status_req(rx_buf))
            {   
                
                make_status_req_msg(rx_buf, tx_buf);

                while (true)
                {
                    
                    if ((micros() - start) > (delay_msg))
                    {
                        if ((micros() - start) > max_frame_delay)
                        {
                            break;
                        }
                        connected = true;    
                        
                        logy((String)("Pre Req Response Time: "+(String)(micros() -s)), 4);
  
                        ser.serial_send(tx_buf);
                        req_resp_time = micros() -s;
                        logy(buffer_to_string(tx_buf.buf,tx_buf.size), 3);
                        logy((String)("Req Response Time: "+(String)req_resp_time), 3);
                        
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
                broadcast_recv = true;
                logy((String)("State Update Time: "+(String)(micros() -s)), 5);
            }
        }
        
    }
}

int Hoermann::set_delay(int delay_)
{
    delay_msg = delay_;
}


void Hoermann::logy(String msg, int level)
{
    if (level == debud_level)
    {
        Serial.println(msg.c_str());
    }
}

void Hoermann::enable_debug(int level)
{
    debud_level = level;
}

String Hoermann::is_connected()
{
    if (connected)
    {
        return (String)"1";
    }
    else
    {
        return (String)"0";
    }
}
String Hoermann::is_scanning()
{
    if (scanning)
    {
        return (String)"1";
    }
    else
    {
        return (String)"0";
    }
}


String Hoermann::is_broadcast_recv()
{
    if (broadcast_recv)
    {
        return (String)"1";
    }
    else
    {
        return (String)"0";
    }
}     

void Hoermann::reset_broadcast()
{
    broadcast_recv = false;
}

void Hoermann::reset_connected()
{
    connected = false;
}

void Hoermann::reset_scanning()
{
    scanning = false;
}

int Hoermann::get_scan_resp_time()
{
    return scan_resp_time;
}

int Hoermann::get_req_resp_time()
{
    return req_resp_time;
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
        if (buf.buf[0] == BROADCAST_ADDR)
        {
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
        return true;
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

String Hoermann::buffer_to_string(uint8_t *buf, int size )
{

    String stream;
    
    if (size > 0)
    {
        stream += size;
        stream += " | ";
        for (int i = 0; i < (int)size; i++)
        {
            stream += " ";
            char str[10];
            sprintf(str,"%x",buf[i]); //converts to hexadecimal base.
            stream += str;
        }
        return stream.c_str();
    }
    return " ";
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
    // if (req_resp_counter > 0)
    // {z
        slave_respone_data = RESPONSE_DEFAULT;
    //     req_resp_counter = 0;
    // }
    // req_resp_counter++;
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
String Hoermann::get_state_hex()
{
    uint8_t buf[1] = {(uint8_t)broadcast_status}; 
    String status = buffer_to_string(buf, 1);
    return status;
}


void Hoermann::set_state(String action)
{
    if (action == "stop" || action == "STOP")
    {
        slave_respone_data = RESPONSE_STOP;
    }
    else if (action == "open" || action == "OPEN")
    {
        slave_respone_data = RESPONSE_OPEN;
    }
    else if (action == "close" || action == "CLOSE")
    {
        slave_respone_data = RESPONSE_CLOSE;
    }
    else if (action == "venting" || action == "VENTING")
    {
        slave_respone_data = RESPONSE_VENTING;
    }
    else if (action == "light" || action == "LIGHT")
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
