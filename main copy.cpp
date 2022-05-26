
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

unsigned long delayTime;

BH1750 lightMeter(0x23);
// Update these with values suitable for your network.

const char* ssid = "Osiris";
const char* password = "?SuperTajneHas!o!@";
const char* mqtt_server = "192.168.1.111";

// GPIO where the DS18B20 is connected to
const int oneWireBus = 2;     
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);


WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),"mqttuser", "mqttuser")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  lightMeter.begin();
  sensors.begin();
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  bool status;

  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  status = bme.begin(0x76);  
  if (!status) {
  Serial.println("Could not find a valid BME280 sensor, check wiring!");
  while (1);
  }
}
void light_meter()
{
  snprintf (msg, MSG_BUFFER_SIZE, "%f", lightMeter.readLightLevel());
  client.publish("esp/light_lux", msg);
}

void bme_read() {
  

  snprintf (msg, MSG_BUFFER_SIZE, "%f", bme.readTemperature());
  client.publish("esp/temperature", msg);

  snprintf (msg, MSG_BUFFER_SIZE, "%f", bme.readPressure()/100);
  client.publish("esp/pressure", msg);

  snprintf (msg, MSG_BUFFER_SIZE, "%f", bme.readHumidity()/100);
  client.publish("esp/humidity", msg);

  snprintf (msg, MSG_BUFFER_SIZE, "%f",bme.readAltitude(1012));
  client.publish("esp/humidity", msg);


}
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 1000) 
  {
    lastMsg = now;

    sensors.requestTemperatures(); 
    float temperatureC = sensors.getTempCByIndex(0);
    float temperatureF = sensors.getTempFByIndex(0);
    Serial.print(temperatureC);
    Serial.println("ºC");
    bme_read();
    light_meter();
    snprintf (msg, MSG_BUFFER_SIZE, "hello world");
    client.publish("outTopic", msg);
    snprintf (msg, MSG_BUFFER_SIZE, "%f", temperatureC);
    client.publish("esp/1w", msg);
  }
}

