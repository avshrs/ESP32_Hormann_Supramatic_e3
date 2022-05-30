#include "serial_w.h"

void SerialW::serial_open(int tx_pin)
{
    Serial2.begin(19200);
    lead_z = 0;
    EnTxPin = tx_pin;
}   


void SerialW::send_brake()
{
    char buf[1] = {0};
    Serial2.begin(9600, SERIAL_7N1);
    Serial2.setTimeout(100);  
    Serial2.flush();
    Serial2.write(buf, 1);
    Serial2.begin(19200, SERIAL_8N1);
    Serial2.setTimeout(100);  
    Serial2.flush();
}

void SerialW::serial_send(TX_Buffer &tx_buffer)
{ 	
    digitalWrite(EnTxPin, HIGH);
    send_brake();
    Serial2.write(tx_buffer.buf, tx_buffer.size);
    digitalWrite(EnTxPin, LOW);
    
}

void SerialW::serial_read(RX_Buffer &rx_buffer)
{	
    uint8_t buf_[10];
    if(Serial2.available() > 0)
    {
        rx_buffer.size =  Serial2.readBytes(rx_buffer.buf, 10);
    }
}


