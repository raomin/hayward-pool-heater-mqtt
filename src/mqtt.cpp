#include <mqtt.h>

// MQTT
WiFiClient espClient;
PubSubClient client(espClient);

void MQTT_reconnect()
{
    int retries = 0;
    // Loop until we're reconnected
    while (!client.connected())
    {
        client.setBufferSize(2048);
        Serial.print("Attempting MQTT connection...");
        if (client.connect(HARDWARE_HOSTNAME, MQTT_USER, MQTT_PASS, "homeassistant/climate/heatpumppool/availability", 0, true, "offline"))
        {
            Serial.println("connected");

            client.subscribe("heatpumppool/set_power_on");
            client.subscribe("heatpumppool/set_power_off");
            client.subscribe("heatpumppool/set_temp");
            client.subscribe("heatpumppool/set_mode");

            client.publish("pool", "connected");
            // Publish the device's information to Home Assistant for automatic discovery
            client.publish("homeassistant/climate/heatpumppool/config","{\"device\": {\"name\": \"HeatPumpPool\", \"identifiers\": [\"heatpumppool\"], \"manufacturer\": \"HeatPumpPool\", \"model\": \"HeatPumpPool\", \"sw_version\": \"1.0\"},\"name\": \"HeatPumpPool Climate\", \"unique_id\": \"heatpumppool_climate\", \"mode_command_topic\": \"heatpumppool/set_mode\", \"mode_state_topic\": \"heatpumppool/mode/state\", \"temperature_command_topic\": \"heatpumppool/set_temp\", \"current_temperature_topic\": \"heatpumppool/temp_in\",\"temperature_state_topic\": \"heatpumppool/temp_prog\", \"modes\": [\"off\", \"heat\", \"cool\", \"auto\"], \"min_temp\": \"10\", \"max_temp\": \"40\", \"temp_step\": \"1\", \"availability_topic\": \"homeassistant/climate/heatpumppool/availability\"}",true);

            // Publish a temp sensor for the pool
            client.publish("homeassistant/sensor/hppool_temp_out/config",
                           "{\"device\": {\"name\": \"HeatPumpPool\", \"identifiers\": [\"heatpumppool\"]},\"name\": \"HeatPump pool Water out Temp\", \"unique_id\": \"heatpumppool_temp_out\", \"state_topic\": \"heatpumppool/temp_out\", \"unit_of_measurement\": \"°C\", \"device_class\": \"temperature\", \"availability_topic\": \"homeassistant/climate/heatpumppool/availability\"}", true);
            client.publish("homeassistant/sensor/hppool_air_out/config",
                           "{\"device\": {\"name\": \"HeatPumpPool\", \"identifiers\": [\"heatpumppool\"]},\"name\": \"HeatPump pool Air temp\", \"unique_id\": \"heatpumppool_air_temp\", \"state_topic\": \"heatpumppool/temp_air\", \"unit_of_measurement\": \"°C\", \"device_class\": \"temperature\", \"availability_topic\": \"homeassistant/climate/heatpumppool/availability\"}", true);
            client.publish("homeassistant/sensor/hppool_condtemp_out/config",
                           "{\"device\": {\"name\": \"HeatPumpPool\", \"identifiers\": [\"heatpumppool\"]},\"name\": \"HeatPump pool Condenser temp\", \"unique_id\": \"heatpumppool_cond_temp\", \"state_topic\": \"heatpumppool/temp_cond\", \"unit_of_measurement\": \"°C\", \"device_class\": \"temperature\", \"availability_topic\": \"homeassistant/climate/heatpumppool/availability\"}", true);

            // Subscribe to the command topic to receive commands from Home Assistant
            client.publish("homeassistant/climate/heatpumppool/availability", "online", true);
            Serial.println("MQTT connected and subscribed");
        }
        else
        {
            retries++;
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
            if (retries > 5)
            {
                ESP.restart();
            }
        }
    }
}

void prepareCmd(){
    //initialize currentCommand with currentStatus
    currentCommand.targetTempCooling = currentStatus.targetTempCooling;
    currentCommand.targetTempHeating = currentStatus.targetTempHeating;
    currentCommand.defrostDuration = currentStatus.defrostDuration;
    currentCommand.defrostTemp = currentStatus.defrostTemp;
    currentCommand.defrostLeaveTemp = currentStatus.defrostLeaveTemp;
    currentCommand.onOff = currentStatus.heatOn;

}

void mqttMsgReceivedCallBack(char *topic, byte *payload, unsigned int length)
{
    payload[length] = '\0';
    if (currentStatus.targetTempCooling==0)
        prepareCmd();
    if (strcmp(topic, "heatpumppool/set_mode") == 0)
    {
        if (strcmp((char *)payload, "auto") == 0)
        {
            // cmdMode = AUTO;
            log_i("Setting mode to auto - not implemented yet");
        }
        else if (strcmp((char *)payload, "cool") == 0)
        {
            // cmdMode = COOL;
            log_i("Setting mode to cool - not implemented yet");
        }
        else if (strcmp((char *)payload, "heat") == 0)
        {
            // cmdMode = HEAT;
            log_i("Setting mode to heat");
            currentCommand.onOff = true;
        }
        else if (strcmp((char *)payload, "off") == 0)
        {
            // cmdPower = false;
            log_i("Setting mode to off");
            currentCommand.onOff = false;
        }
    }
    else if (strcmp(topic, "heatpumppool/set_temp") == 0)
    {
        log_i("Setting temp to %s", payload);
        String s = String((char *)payload);
        int temp = s.toInt();

        if (temp >= 10 && temp <= 30 && currentCommand.mode == COOL)
        {
            currentCommand.targetTempCooling = temp;
        }
        else if (temp >= 10 && temp <= 30 && currentCommand.mode == HEAT)
        {
            currentCommand.targetTempHeating = temp;
        }
    }
    sendCommand();
}



void publishCurrentState()
{
    log_i("Publishing current state to MQTT");
    client.publish("heatpumppool/mode/state", currentStatus.heatOn ? "heat" : "off");
    client.publish("heatpumppool/temp_in", String(currentStatus.intakeWaterTempSensor).c_str());
    client.publish("heatpumppool/temp_out", String(currentStatus.exhaustWaterTempSensor).c_str());
    client.publish("heatpumppool/temp_cond", String(currentStatus.condenserTempSensor).c_str());
    client.publish("heatpumppool/temp_air", String(currentStatus.externalTemp).c_str());
    if (currentStatus.heatOn)
    {
        client.publish("heatpumppool/temp_prog", String(currentStatus.targetTempHeating).c_str());
    }
    else
    {
        client.publish("heatpumppool/temp_prog", String(currentStatus.targetTempCooling).c_str());
    }
}

void mqttSetup()
{
    client.setServer(MQTT_HOST, 1883);
    client.setCallback(mqttMsgReceivedCallBack);
    MQTT_reconnect();
}

void mqttLoop()
{
     if (!client.connected())
    {
        MQTT_reconnect();
    }
    client.loop();
}
void publishRawFrame(volatile byte *buffer){
    char payload[50];
    sprintf(payload, "%d,%d,%d,%d,%d,%d,%d,%d,%d", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8]);
    client.publish("heatpumppool/rawframe", payload);
}