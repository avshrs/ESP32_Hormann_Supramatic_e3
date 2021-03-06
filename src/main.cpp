#include <string.h>
#include <Wire.h>
#include "passwd.h"
#include "wifi_mqtt.h"
#include "hoermann.h"


int d = 1; 
unsigned long previousMillis = 0;  
unsigned long previousMillis2 = 0;  
Hoermann hoermann; 
String state = "n/a";


void callback(char* topic, byte* payload, unsigned int length) 
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    String st;
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
        st +=(char)payload[i];
    }
    Serial.println();

    
    if (strcmp(topic,"avshrs/devices/hormann_garage_door_01/set/debug") == 0)  
    {   
        hoermann.enable_debug(atoi((char *)payload));
    } 
    else if (strcmp(topic,"avshrs/devices/hormann_garage_door_01/set/door") == 0)  
    {   
        Serial.println(st);
        hoermann.set_state(st);
    } 
    else if (strcmp(topic,"avshrs/devices/hormann_garage_door_01/set/venting") == 0)  
    {   
        Serial.println(st);
        hoermann.set_state(st);
    } 
    else if (strcmp(topic,"avshrs/devices/hormann_garage_door_01/set/delay_msg") == 0)  
    {   
        Serial.print("delay_msg: ");
        Serial.println(st);
        hoermann.set_delay(st.toInt());
    } 
    else if (strcmp(topic,"avshrs/devices/hormann_garage_door_01/set/light") == 0)  
    {   
        if (st == "ON" || st == "OFF")
            hoermann.set_state("light");
    }
    else if (strcmp(topic,"avshrs/devices/hormann_garage_door_01/esp_led") == 0 && (char)payload[0] == '1') 
    {
        Serial.println("BUILTIN_LED_low");
        digitalWrite(BUILTIN_LED, LOW);   
    } 
    else if (strcmp(topic,"avshrs/devices/hormann_garage_door_01/esp_led") == 0 && (char)payload[0] == '0') 
    {
        Serial.println("BUILTIN_LED_high");
        digitalWrite(BUILTIN_LED, HIGH); 
    }


}

void setup() 
{
    int EnTxPin =  18;
    Wire.begin();
    hoermann.init(EnTxPin);

    Serial.begin(1000000);
    
    pinMode(EnTxPin, OUTPUT);  
    digitalWrite(EnTxPin, LOW);
    pinMode(BUILTIN_LED, OUTPUT);  
    digitalWrite(BUILTIN_LED, LOW);   
    setup_wifi();

    client.setServer(mqtt_server, 1883);
    client.setBufferSize(1024);
    client.setCallback(callback);
}

void door_position(boolean force)
{
    if (state != hoermann.get_state() || force ) 
    {
        state = hoermann.get_state();
        
        if (hoermann.get_state() == "open")
        {
            client.publish("avshrs/devices/hormann_garage_door_01/state/door", "100");
            client.publish("avshrs/devices/hormann_garage_door_01/state/state", "open");
        }
        else if (hoermann.get_state() == "closed")
        {
            client.publish("avshrs/devices/hormann_garage_door_01/state/door", "0");
            client.publish("avshrs/devices/hormann_garage_door_01/state/venting", "0");
            client.publish("avshrs/devices/hormann_garage_door_01/state/state", "closed");
        }
        else if (hoermann.get_state() == "venting")
        {
            client.publish("avshrs/devices/hormann_garage_door_01/state/door", "10");
            client.publish("avshrs/devices/hormann_garage_door_01/state/venting", "10");
            client.publish("avshrs/devices/hormann_garage_door_01/state/state", "venting");
        }
        else
        {
            client.publish("avshrs/devices/hormann_garage_door_01/state/door", "error");
        }
    }
}

void loop() 
{
    currentMillis = millis();

    if (!client.connected()) 
    {
        reconnect();
    }
    client.loop();
    hoermann.run_loop();
    
    if (currentMillis - previousMillis >= 60000) 
    {
        previousMillis = currentMillis;

        wifi_status();

        snprintf (msg, MSG_BUFFER_SIZE, "true");
        client.publish("avshrs/devices/hormann_garage_door_01/status/connected", msg);

        client.publish("avshrs/devices/hormann_garage_door_01/status/master_sending_broadcast", hoermann.is_broadcast_recv().c_str());
        hoermann.reset_broadcast();

        client.publish("avshrs/devices/hormann_garage_door_01/status/master_is_scanning", hoermann.is_scanning().c_str());
        hoermann.reset_scanning();

        client.publish("avshrs/devices/hormann_garage_door_01/status/response_to_master", hoermann.is_connected().c_str());
        hoermann.reset_connected();

        snprintf (msg, MSG_BUFFER_SIZE, "%i", hoermann.get_scan_resp_time());
        client.publish("avshrs/devices/hormann_garage_door_01/status/scan_resp_time", msg);
        
        snprintf (msg, MSG_BUFFER_SIZE, "%i", hoermann.get_req_resp_time());
        client.publish("avshrs/devices/hormann_garage_door_01/status/req_resp_time", msg);
        client.publish("avshrs/devices/hormann_garage_door_01/state/light", "OFF");
        door_position(true);        
    }
    
    door_position(false);        
    
}