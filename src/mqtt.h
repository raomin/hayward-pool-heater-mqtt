#pragma once
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <common.h>
#include <comm2.h>



extern void sendCommand();
void MQTT_reconnect();
void mqttMsgReceivedCallBack(char *topic, byte *payload, unsigned int length);
void publishCurrentState();
void prepareCmd();
void mqttSetup();
void mqttLoop();
void publishRawFrame(volatile byte *buffer);

