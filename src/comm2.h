#pragma once
#include <Arduino.h>
#include <common.h>
#include <mqtt.h>


#define COOLTEMP_ADDR 1
#define HEATTEMP_ADDR 2
#define DEFROSTDURATION_ADDR 3
#define DEFROSTTEMP_ADDR 4
#define DEFROSTLEAVETEMP_ADDR 5

byte reverseBits(byte x);
void _sendHigh(u32_t us);
void _sendLow(u32_t us);
void _sendByte(byte b);
void sendFrame();
void sendCommand();
void readBuffer(volatile byte *buffer);

