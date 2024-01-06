#pragma once
#include <Arduino.h>

#if __has_include("config.h")
#include "config.h"
#else
#include "my_config.h"
#endif


#define COOL B00000000
#define HEAT B00001000
#define AUTO B00000100

struct commandData
{
    byte targetTempCooling;
    byte targetTempHeating;
    byte defrostDuration;
    byte defrostTemp;
    byte defrostLeaveTemp;
    byte mode;
    bool onOff;
};

extern commandData currentCommand;

struct sensorData
{
    int targetTempCooling;
    int targetTempHeating;
    int defrostDuration;
    int defrostTemp;
    int defrostLeaveTemp;
    int numberOfSystems;
    bool automaticRestart;
    bool heatOn;
    float intakeWaterTempSensor;
    int exhaustWaterTempSensor;
    int condenserTempSensor;
    int externalTemp;
    bool canceling;
    uint8_t mode;
};


extern sensorData currentStatus;

