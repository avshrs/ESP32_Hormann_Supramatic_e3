#include <stdint.h>
#include <stdbool.h>
#include "hoermann.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <unistd.h>
#include <algorithm>    // std::fill

#include <ctime>   // localtime
#include <stdlib.h>
#include <sstream> // stringstream




void Hoermann::init(String * state_, int tx_pin)
{
    state = state_;

    ser.serial_open(tx_pin);
}


void Hoermann::run_loop(void)
{   
    
    unsigned long start = millis();
    RX_Buffer rx_buf;
    TX_Buffer tx_buf;


        rx_buf.buf.clear();
        tx_buf.buf.clear();
        ser.serial_read(rx_buf);
        // print_buffer(rx_buf.buf.data(), rx_buf.buf.size());

        start = millis();
        if(is_frame_corect(rx_buf))
        {     
            if(is_broadcast(rx_buf))
            {
                if(is_broadcast_lengh_correct(rx_buf))
                    {
                        // print_buffer(rx_buf.buf.data(), rx_buf.buf.size());
                        update_broadcast_status(rx_buf);
                    }
            }

            else if(is_slave_query(rx_buf))
            {   
                if(is_slave_scan(rx_buf))
                {
                    
                    make_scan_responce_msg(rx_buf, tx_buf);
                    while(true)
                    {
                        
                        if((millis() - start) > (tx_buf.timeout) )
                        {   
                            if((millis() - start) > max_frame_delay)
                            {
                                break;
                            }
                            // print_buffer(rx_buf.buf.data(), rx_buf.buf.size());
                            ser.serial_send(tx_buf);
                            
                            break;
                            
                        }

                        usleep(10);
                    }                    
                    
                }    
                else if(is_slave_status_req(rx_buf))
                {
                    make_status_req_msg(rx_buf, tx_buf);
                    while(true)
                    {
                        
                        
                        if( (millis() - start) > (tx_buf.timeout))
                        {   
                            if((millis() - start) > max_frame_delay)
                            {
                                break;
                            }
                            ser.serial_send(tx_buf);
                            break;
                        }

                        usleep(10);
                    }
                }
            }
        }
}       


void Hoermann::update_broadcast_status(RX_Buffer &buf)
{
;
  
  uint8_t br = buf.buf.at(2);
  if (static_cast<uint8_t>(broadcast_status) != br)
  {
    broadcast_status = static_cast<uint16_t>(br);
  
  
    *state = get_state();
    
    }
}


uint8_t Hoermann::get_length(RX_Buffer &buf)
{   
    if(buf.buf.size() > 2)
    {
        return buf.buf.at(1) & 0x0F;
    }
    else
        return 0x00;
}

uint8_t Hoermann::get_counter(RX_Buffer &buf)
{
    if(buf.buf.size() > 2)
    {
        return (buf.buf.at(1) & 0xF0) + 0x10;
    }
    else
        return 0x00;

}

bool Hoermann::is_broadcast(RX_Buffer &buf)
{   
    if(buf.buf.size() == 5)
    {
        if(buf.buf.at(0) == BROADCAST_ADDR && calc_crc8(buf.buf.data(), 4) == buf.buf.at(4))
        {   
            // print_buffer(buf.buf.data(), buf.buf.size());
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
    if(buf.buf.size() > 3 && buf.buf.size() < 6 )
    {
        if(buf.buf.at(0) == UAP1_ADDR)
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
    if(buf.buf.size() > 3 && buf.buf.size() < 6)
    {
        if(calc_crc8(buf.buf.data(), buf.buf.size()-1) == buf.buf.at(buf.buf.size()-1) )
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
    if(buf.buf.size() == 5)
    {
        if(is_broadcast_lengh_correct(buf) && (buf.buf.at(2) == CMD_SLAVE_SCAN))
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
    if(buf.buf.size() == 4)
    {    
    if(is_req_lengh_correct(buf) && (buf.buf.at(2) == CMD_SLAVE_STATUS_REQUEST))
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
    if(get_length(buf) == broadcast_lengh)
        return true;
    else
        return false;
}

bool Hoermann::is_req_lengh_correct(RX_Buffer &buf)
{
    if(get_length(buf) == reguest_lengh)
        return true;
    else
        return false;
}


void Hoermann::print_buffer(uint8_t *buf, int len)
{   
    for(int i = 0; i < len  ; i++)
        {
        Serial.print(buf[i], HEX);
        }
     Serial.println();
}

uint8_t Hoermann::get_master_address()
{
    return master_address;
}

void Hoermann::make_scan_responce_msg(RX_Buffer &rx_buf, TX_Buffer &tx_buf)
{
    tx_buf.buf.push_back(get_master_address());
    tx_buf.buf.push_back(0x02 | get_counter(rx_buf));
    tx_buf.buf.push_back(UAP1_TYPE);
    tx_buf.buf.push_back(UAP1_ADDR);
    tx_buf.buf.push_back(calc_crc8(tx_buf.buf.data(), 4));
    tx_buf.timeout = 2000;
}

void Hoermann::make_status_req_msg(RX_Buffer &rx_buf, TX_Buffer &tx_buf)
{
    tx_buf.buf.push_back(get_master_address());
    tx_buf.buf.push_back(0x03 | get_counter(rx_buf));

    tx_buf.buf.push_back(CMD_SLAVE_STATUS_RESPONSE);
    if(slave_respone_data == RESPONSE_STOP)
    {
        tx_buf.buf.push_back(0x00);
        tx_buf.buf.push_back(0x00);
    }
    else 
    {
        tx_buf.buf.push_back(static_cast<uint8_t>(slave_respone_data));
        tx_buf.buf.push_back(0x10);   
    }
    slave_respone_data = RESPONSE_DEFAULT;
    tx_buf.buf.push_back(calc_crc8(tx_buf.buf.data(), 5));
    tx_buf.timeout = 2000;
}


String Hoermann::get_state()
{
  if ((broadcast_status) == 0x00)
  {
    return (String)"venting";
  }
  else if ((broadcast_status) == 0x02)
  {
    return (String)"closed";
  }
  else if ((broadcast_status) == 0x01)
  {
    return (String)"open";
  }
    else
    return (String)"error";
 
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
    if(action == "stop")
    {
      slave_respone_data = RESPONSE_STOP;
    }
    else if(action == "open")
    {
      slave_respone_data = RESPONSE_OPEN;
    }
    else if(action == "close")
    {
      slave_respone_data = RESPONSE_CLOSE;
    }
    else if(action == "venting")
    {
      slave_respone_data = RESPONSE_VENTING;
    }
    else if(action == "light")
    {
      slave_respone_data = RESPONSE_TOGGLE_LIGHT;
    }
    
}



uint8_t Hoermann::calc_crc8(uint8_t *p_data, uint8_t len)
{
size_t i;
uint8_t crc = CRC8_INITIAL_VALUE;
    while(len--){
        crc ^= *p_data++;
        for(i = 0; i < 8; i++){
            if(crc & 0x80){
                crc <<= 1;
                crc ^= 0x07;
            } else {
                crc <<= 1;
            }
        }
    }
    
    return(crc);
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
    set_state( "stop");
}

