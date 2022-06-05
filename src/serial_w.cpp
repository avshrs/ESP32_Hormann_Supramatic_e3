#include "serial_w.h"

void SerialW::serial_open(int tx_pin)
{
    Serial2.begin(19200);
    lead_z = 0;
    EnTxPin = tx_pin;
    pinMode(EnTxPin, OUTPUT);  
    digitalWrite(EnTxPin, LOW);
}   

void SerialW::send_brake()
{
    char buf[1] = {0};
    
    Serial2.updateConfig(9600, SERIAL_7N1);
    
    while(!Serial2.availableForWrite()){}
    
    Serial2.write(buf, 1);
    
    Serial2.updateConfig(19200, SERIAL_8N1);



}

void SerialW::serial_send(TX_Buffer &tx_buffer)
{ 	
    digitalWrite(EnTxPin, HIGH);

    send_brake();

    uint8_t d[1] = {0}; 
    
    Serial2.flush();

    while(!Serial2.availableForWrite()){}
    Serial2.write(tx_buffer.buf, tx_buffer.size);   

    usleep(3500);

    Serial2.write(d, 1);  

    digitalWrite(EnTxPin, LOW);
} 

void SerialW::serial_read(RX_Buffer &rx_buffer)
{	
    uint8_t buf = 0x00;
    buf = Serial2.read();
    if(buf == 0x00)
    {
        while(!(Serial2.available())){}
        buf = Serial2.read();
        rx_buffer.buf[0] =  buf;
        if(buf == 0x28)
        {
            // v1 & 0x0f
            while(!(Serial2.available())){}
            buf = Serial2.read();
            rx_buffer.buf[1] =  buf;
            if((buf & 0x0f) == 0x02)
            {
                for( int i = 0 ; i<3; i++ )
                { 
                    while(!(Serial2.available())){}
                    buf = Serial2.read();
                    rx_buffer.buf[2+i] =  buf;
                }
                rx_buffer.size = 5;
            }
            else if((buf & 0x0f) == 0x01)
            {
                for( int i = 0 ; i<2; i++ )
                { 
                    while(!(Serial2.available())){}
                    buf = Serial2.read();
                    rx_buffer.buf[2+i] =  buf;
                }
                rx_buffer.size = 4;
            }
        }
        else if(buf == 0x00)
        {
            while(!(Serial2.available())){}
            
            buf = Serial2.read();
            rx_buffer.buf[1] =  buf;
            if (buf == 0x02 || buf == 0x82 )
            {
                for( int i = 0 ; i<3; i++ ) 
                {
                    while(!(Serial2.available())){}
                    buf = Serial2.read();
                    rx_buffer.buf[2+i] =  buf;
                }
                rx_buffer.size = 5;
            }
            
        }
    
    }
    
}


