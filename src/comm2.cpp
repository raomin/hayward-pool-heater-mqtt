#include "comm2.h"
#include "listener.h"


#define MODE_HEAT 0
#define MODE_COOL 1
#define MODE_AUTO 2
#define MODE_OFF 3


sensorData currentStatus = {
    .targetTempCooling = 22,
    .targetTempHeating = 26,
    .defrostDuration = 45,
    .defrostTemp = 7,
    .defrostLeaveTemp = 13,
    .numberOfSystems = 0,
    .automaticRestart = false,
    .heatOn = false,
    .intakeWaterTempSensor = 10,
    .exhaustWaterTempSensor = 10,
    .condenserTempSensor = 10,
    .externalTemp = 10,
    .canceling = false,
    .mode = MODE_AUTO
    };

commandData currentCommand;

byte cmdFrame[9] = {
    204, // 0 - HEADER
    22,  // cooling temp
    36,  // Heating temp
    45,  // Defrost duration
    7,   // Defrost temp (minus)
    13,  // temp to leave defrost
    80,
    60,
    0, // 9 - CHECKSUM
};

// Reverse the order of bits in a byte.
// I.e. MSB is swapped with LSB, etc.
byte reverseBits(byte x)
{
    x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
    x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
    x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);
    return x;
}

void _sendHigh(u32_t us)
{
    digitalWrite(PIN_NET, HIGH);
    delayMicroseconds(us);
}

void _sendLow(u32_t us)
{
    digitalWrite(PIN_NET, LOW);
    delayMicroseconds(us);
    digitalWrite(PIN_NET, HIGH);
}

void _sendByte(byte b) // send a byte starting with the LSB
{
    for (byte i = 0; i < 8; i++)
    {
        if (b & 1) // if the bit is 1 send short pulse
        {
            _sendHigh(2800);
            _sendLow(1000);
        }
        else // if the bit is 0 send long pulse
        {
            _sendHigh(1000);
            _sendLow(1000);
        }
        b >>= 1; // shift the bits to the right
    }
}

void sendFrame()
{
    // just detach interrupt while we send
    detachInterrupt(digitalPinToInterrupt(PIN_NET));
    log_i("Sending frames");

    // reverse all bits in the frame
    //  for (byte i = 0; i < 9; i++)
    //  {
    //      commFrame[i] = reverseBits(cmdFrame[i]);
    //  }

    pinMode(PIN_NET, OUTPUT);

    // send 4 frames
    for (byte j = 0; j < 4; j++)
    {
        // start with LOW for 8.3ms
        _sendLow(8200);
        // then a HIGH for 4.1ms
        _sendHigh(4200);
        _sendLow(1000);

        // Send the frame
        for (byte i = 0; i < 9; i++) // send all 9 bytes
        {
            _sendByte(cmdFrame[i]);
        }
        _sendHigh(237000);
    }
    pinMode(PIN_NET, INPUT);
    // reattach interrupt
    attachInterrupt(digitalPinToInterrupt(PIN_NET), listener_interrupt, CHANGE);
}

void sendCommand()
{
    // update cmdFrame with currentCommand
    cmdFrame[COOLTEMP_ADDR] = currentCommand.targetTempCooling;
    cmdFrame[HEATTEMP_ADDR] = currentCommand.targetTempHeating;
    cmdFrame[DEFROSTDURATION_ADDR] = currentCommand.defrostDuration;
    cmdFrame[DEFROSTTEMP_ADDR] = currentCommand.defrostTemp;
    cmdFrame[DEFROSTLEAVETEMP_ADDR] = currentCommand.defrostLeaveTemp;
    cmdFrame[7] = 60;
    // heating ON/OFF is second bit of byte 8
    if (currentCommand.onOff)
    {
        cmdFrame[7] |= 0b01000000;
    }
    else
    {
        cmdFrame[7] &= 0b10111111;
    }

    log_i("Sending command:");
    log_i("Cooling temp: %d", cmdFrame[COOLTEMP_ADDR]);
    log_i("Heating temp: %d", cmdFrame[HEATTEMP_ADDR]);
    log_i("Defrost duration: %d", cmdFrame[DEFROSTDURATION_ADDR]);
    log_i("Defrost temp: %d", cmdFrame[DEFROSTTEMP_ADDR]);
    log_i("Defrost leave temp: %d", cmdFrame[DEFROSTLEAVETEMP_ADDR]);
    log_i("On/Off: %d", currentCommand.onOff);

    // calculate checksum: sum of all bytes except header
    byte checksum = 0;
    for (byte i = 1; i < 8; i++)
    {
        checksum += cmdFrame[i];
    }
    cmdFrame[8] = checksum;
    sendFrame();
}

/////////////////////////////////
////// READING BOARD FRAME //////
/////////////////////////////////

int stateCounter = 0;
void readBuffer(volatile byte *buffer)
{

    // check if frame is  A or B, update currentStatus
    if (buffer[0] == 210)
    { //
        // frame A
        log_i("Frame A");
        log_i("Target temp cooling: %d", buffer[1]);
        currentStatus.targetTempCooling = buffer[1];
        currentCommand.targetTempCooling = buffer[1];
        log_i("Target temp heating: %d", buffer[2]);
        currentStatus.targetTempHeating = buffer[2];
        currentCommand.targetTempHeating = buffer[2];
        log_i("Defrost cycle length: %d", buffer[3]);
        currentStatus.defrostDuration = buffer[3];
        currentCommand.defrostDuration = buffer[3];
        log_i("Defrost start temp trigger: %d", buffer[4]);
        currentStatus.defrostTemp = buffer[4];
        currentCommand.defrostTemp = buffer[4];
        log_i("Defrost stop temp trigger: %d", buffer[5]);
        currentStatus.defrostLeaveTemp = buffer[5];
        currentCommand.defrostLeaveTemp = buffer[5];
        log_i("Heat on: %d", (buffer[7] & 0b01000000) != 0);
        currentStatus.heatOn = (buffer[7] & 0b01000000) != 0;
        currentCommand.onOff = (buffer[7] & 0b01000000) != 0;

    }
    else if (buffer[0] == 221)
    {
        // frame B
        log_i("Frame B");
        log_i("Intake water temp sensor: %d", buffer[1]);
        currentStatus.intakeWaterTempSensor = buffer[1];
        log_i("Exhaust water temp sensor: %d", buffer[2]);
        currentStatus.exhaustWaterTempSensor = buffer[2];
        log_i("Condenser temp sensor: %d", buffer[3]);
        currentStatus.condenserTempSensor = buffer[3];
        log_i("Canceling: %d", buffer[4] & 0b00000001 == 0b00000001);
        currentStatus.canceling = buffer[4] & 0b00000001 == 0b00000001;
        log_i("External temp: %d", buffer[5]);
        currentStatus.externalTemp = buffer[5];
        // check if we have .5
        if (buffer[9] & 0b10000000 != 0)
        {
            log_i("intakeWaterTempSensor is .5");
            currentStatus.intakeWaterTempSensor += 0.5;
        }
    }
    else
    {
        log_i("Unknown frame");
        // send frame to mqtt for debugging
        publishRawFrame(buffer);
        // display frame on serial for debugging
        return;
    }
    stateCounter++;
    if (stateCounter == 2)
    { // we have both frames, publish to MQTT
        stateCounter = 0;
        publishCurrentState();
    }
}