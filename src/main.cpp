#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <comm2.h>
#include "common.h"
#include <mqtt.h>
#include <listener.h>
#include <ArduinoOTA.h>
#include <WiFiMulti.h>

WiFiMulti WiFiMulti;

void wifiSetup()
{

    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);
    WiFiMulti.addAP(WIFI_SSID, WIFI_PASS);

    

    uint16_t timeout = millis() + 60000;
    while(WiFiMulti.run() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
        Serial.print(".");
        if (millis() > timeout)
        {
            ESP.restart();
        }
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void otaStarted()
{
    Serial.println("OTA started");
    // Disable the listener interrupt while we update the firmware
    detachInterrupt(digitalPinToInterrupt(PIN_NET));
}

void otaEnded()
{
    Serial.println("OTA ended");
    // Re-enable the listener interrupt
    attachInterrupt(digitalPinToInterrupt(PIN_NET), listener_interrupt, CHANGE);
}


void setup()
{
    Serial.begin(115200);
    Serial.println("Start");
    wifiSetup();
    mqttSetup();
    listenerSetup();
    onBuffer = readBuffer;//set the callback function for the listener
    // OTA setup
    ArduinoOTA.begin();
    ArduinoOTA.setHostname(HARDWARE_HOSTNAME);
    ArduinoOTA.onStart(otaStarted);
    ArduinoOTA.onEnd(otaEnded);
    ArduinoOTA.onError([](ota_error_t error) {
        otaEnded();
        ESP.restart();
    });

    Serial.println("Setup completed");
}

void loop()
{
    mqttLoop();
    listenerLoop();
    // Handle OTA events
    ArduinoOTA.handle();
}
