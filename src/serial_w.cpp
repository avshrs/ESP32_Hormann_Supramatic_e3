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
    Serial2.write(tx_buffer.buf.data(), tx_buffer.buf.size());
    digitalWrite(EnTxPin, LOW);
    
}

void SerialW::serial_read(RX_Buffer &rx_buffer)
{	
  uint8_t buf[20] = {0};
  int s = Serial2.read(buf, sizeof(buf));
  if(s>3)
  {
      for(int i=0+lead_z; i < s; i++)
      {
          rx_buffer.buf.push_back(buf[i]);
      }
  } 
}


