#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <stdint.h>
#include <string>
#include "vars.h"

class SerialW
{
  public:
    int fd;
    int lead_z = 0 ;
    int EnTxPin =  0;  // HIGH:Transmitter, LOW:Receiver
     

    void serial_open(int tx_pin);
    void serial_send(TX_Buffer &tx_buffer);
    void serial_read(RX_Buffer &rx_buffer);
    void send_brake();
};

