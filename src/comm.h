#include <Arduino.h>

#if __has_include("config.h")
#include "config.h"
#else
#include "my_config.h"
#endif

byte defaultMaskPowerMode = 96;
byte defaultMaskTemp = 2;


// static CMD trame values
unsigned char cmdTrame[12] = {
    129, // 0 - HEADER
    141,
    232, // 2 - POWER &MODE
    6,
    238, // 4 - TEMP
    30,
    188,
    188,
    188,
    188,
    0,
    0, // 11 - CHECKSUM
};

byte state = 0;
byte highCpt = 0;
byte buffer = 0;
byte cptBuffer = 0;
byte wordCounter = 0;
byte trame[40] = {};


float cmdTemp;
byte cmdMode;
boolean cmdPower;


// Reverse the order of bits in a byte.
// I.e. MSB is swapped with LSB, etc.
byte reverseBits(unsigned char x)
{
    x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
    x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
    x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);
    return x;
}



void _sendHigh(word ms)
{
    digitalWrite(PIN, HIGH);
    delayMicroseconds(ms * 1000);
}

void _sendLow(word ms)
{
    digitalWrite(PIN, LOW);
    delayMicroseconds(ms * 1000);
}

void sendBinary0()
{
    _sendLow(1);
    _sendHigh(1);
}

void sendBinary1()
{
    _sendLow(1);
    _sendHigh(3);
}

void sendHeaderCmdTrame()
{
    _sendLow(9);
    _sendHigh(5);
}

// between trame repetition
void sendSpaceCmdTrame()
{
    _sendLow(1);
    _sendHigh(100);
}

// after the 8th trame sent
void sendSpaceCmdTramesGroup()
{
    _sendLow(1);
    // to avoid software watchdog reset due to the long 2000ms delay, we cut the 2000ms in 4x500ms and feed the wdt each time.
    for (byte i = 0; i < 4; i++)
    {
        _sendHigh(500);
        // ESP.wdtFeed();
        yield();
    }
}

bool setTempInTrame(float temperature)
{
    byte temp = temperature;
    bool halfDegree = ((temperature * 10) - (temp * 10)) > 0;

    if (temp < 15 || temp > 40)
    {
        Serial.println("Error setTemp: Value must be between 15 & 33");
        return false;
    }

    byte value = temp - 2;
    value = reverseBits(value);
    value = value >> 1;
    cmdTrame[4] = cmdTrame[4] | value;
    if (halfDegree)
    {
        cmdTrame[4] = cmdTrame[4] | B10000000;
    }
    return true;
}

bool setModeInTrame(byte mode)
{
    byte mask;

    switch (mode)
    {
    case HEAT:
        mask = HEAT;
        break;
    case COOL:
        mask = COOL;
        break;
    case AUTO:
        mask = AUTO;
        break;
    default:
        Serial.println("Error setMode: Unknown mode");
        return false;
    }
    cmdTrame[2] = cmdTrame[2] | mask;
    return true;
}

bool setPowerInTrame(bool power)
{
    byte mask = power ? B10000000 : B00000000;
    cmdTrame[2] = cmdTrame[2] | mask;
    return true;
}

byte generateChecksumInTrame()
{
    unsigned int total = 0;
    for (byte i = 0; i < 11; i++)
    {
        total += reverseBits(cmdTrame[i]);
    }
    byte checksum = total % 256;
    checksum = reverseBits(checksum);
    cmdTrame[11] = checksum;
    return checksum;
}

// void printCmdTrame() {
//   Serial.println("--------------------");
//   for(byte i=0; i<=11; i++) {
//     Serial.println(cmdTrame[i], BIN);
//   }
//   Serial.println("--------------------");
// }

byte checksum(byte trame[], byte size)
{
    int total = 0;
    for (int i = 0; i < (size - 1); i++)
    {
        total += reverseBits(trame[i]);
    }
    return total % 256;
}

bool checksumIsValid(byte trame[], byte size)
{
    int value = 0;
    value = reverseBits(trame[size - 1]) - checksum(trame, size);
    return value == 0;
}

void resetRecevingTrameProcess()
{
    pinMode(PIN, INPUT);
    state = 0;
    highCpt = 0;
    cptBuffer = 0;
    buffer = 0;
    wordCounter = 0;
    memset(trame, 0, sizeof(trame));
}

void sendCmdTrame()
{

    pinMode(PIN_NET, OUTPUT);
    // repeat the trame 8 times
    for (byte occurrence = 0; occurrence < 8; occurrence++)
    {
        yield();
        sendHeaderCmdTrame();
        for (byte trameIndex = 0; trameIndex <= 11; trameIndex++)
        {
            byte value = cmdTrame[trameIndex];
            for (byte bitIndex = 0; bitIndex < 8; bitIndex++)
            {
                byte bit = (value << bitIndex) & B10000000;
                if (bit)
                {
                    sendBinary1();
                }
                else
                {
                    sendBinary0();
                }
            }
        }

        if (occurrence < 7)
        {
            sendSpaceCmdTrame();
        }
        else
        {
            sendSpaceCmdTramesGroup();
            Serial.println("Cmd trame sent");
        }
    }
}



void resetTempAndPowerModeMask()
{
    cmdTrame[2] = defaultMaskPowerMode;
    cmdTrame[4] = defaultMaskTemp;
}


void prepareCmdTrame()
{
    resetTempAndPowerModeMask();
    setPowerInTrame(cmdPower);
    setModeInTrame(cmdMode);
    setTempInTrame(cmdTemp);
    generateChecksumInTrame();
}