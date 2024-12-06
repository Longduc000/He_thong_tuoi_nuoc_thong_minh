#include <Arduino.h>
#include "secrets/wifi.h"
#include "wifi_connect.h"
#include <WiFiClientSecure.h>
#include "ca_cert.h"
#include "secrets/mqtt.h"
#include <PubSubClient.h>
#include <Ticker.h>

float moisture;
int sensor_analog;
const int sensor_pin = A0; // soil moisture sensor O/P oin
const int relay = 17;

namespace
{
    const char *ssid = WiFiSecrets::ssid;
    const char *password = WiFiSecrets::pass;
    const char *moisture_topic = "esp32/moisture";
    unsigned int publish_count = 0;
    
    uint16_t keepAlive = 15;    // seconds (default is 15)
    uint16_t socketTimeout = 5; // seconds (default is 15)
}

WiFiClientSecure tlsClient;
PubSubClient mqttClient(tlsClient);

Ticker mqttPulishTicker;

void mqttPublish()
{  
    sensor_analog = analogRead(sensor_pin);
    moisture = (100 - ((sensor_analog / 4095.00) *100));
    Serial.print("Moisture = " );
    Serial.print(moisture); // in ra man hinh nhiet do
    Serial.println("%");
    mqttClient.publish(moisture_topic, String(moisture).c_str(), false);
    if (moisture < 50) {
    digitalWrite(relay, LOW);
    }
    else {
        digitalWrite(relay, HIGH);
    }
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    Serial.printf("From %s:  ", topic);
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

void mqttReconnect()
{
    while (!mqttClient.connected())
    {
        Serial.println("Attempting MQTT connection...");
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        if (mqttClient.connect(client_id.c_str(), MQTT::username, MQTT::password))
        {
            Serial.print(client_id);
            Serial.println(" connected");
            mqttClient.subscribe(moisture_topic);
        }
        else
        {
            Serial.print("MTTT connect failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 1 seconds");
            delay(1000);
        }
    }
}

void setup()
{
    Serial.begin(115200);
    delay(10);
    setup_wifi(ssid, password);
    tlsClient.setCACert(ca_cert);

    // mqttClient.setKeepAlive(keepAlive); // To see how long mqttClient detects the TCP connection is lost
    // mqttClient.setSocketTimeout(socketTimeout); // To see how long mqttClient detects the TCP connection is lost

    mqttClient.setCallback(mqttCallback);
    mqttClient.setServer(MQTT::broker, MQTT::port);
    mqttPulishTicker.attach(1, mqttPublish);
    pinMode(relay, OUTPUT);
    digitalWrite(relay, HIGH);
}

void loop()
{
    if (!mqttClient.connected())
    {
        mqttReconnect();
    }
    mqttClient.loop();
}
