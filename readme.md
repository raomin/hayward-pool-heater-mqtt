# Hayward pool heater MQTT bridge

This is a little project I've been working for a while now. I'm the owner of a Hayward pool heater (*Trevium HP55TR, which is also the same exact model of Hayward energy line pro* )
This heat pump use a controller called **PC1000**.

I have decoded the data using a small logic sniffer.

The last version of the sketch can now **received current parameters** and **send command** to the heatpump.
This version of the sketch is working on a **wemos d1 mini** (using the arduino IDE with **arduino core** installed)

Shematic

You have to connect the `NET` pin of the PC1000 controller to your esp8266 via a *bidirectional* level shifter, and connect the PC1000 `GND` to the `GND` of your esp8266.
The 5v <-> 3.3v level shifter is mandaroty because the esp8266 is not 5V tolerant, and the heatpump **controller is not working with 3.3v**.



**MQTT topics**

Data will be published on your MQTT server every few seconds using this topics:

- `pool/power`  (true / false)
- `pool/mode` (heat / cool)
- `pool/automatic_mode` (true / false) Automatic = heat or cold according to the programmed temp and the out temperature
- `pool/temp_out`  (temperature out in celcius)
- `pool/temp_prog`  (programmed temperature in celcius)

You will be able to change the settings via this topics:

- `pool/set_power_on`  (NULL msg)
- `pool/set_power_off` (NULL msg)
- `pool/set_mode` (HEAT/COOL/AUTO)
- `pool/set_temp`  (temperature in celcius. You could set half degree. Ex: 27.5)

---------

Special thx to the french arduino comumnity, and especially to plode.

[Whole reverse engineering topic (in french)](https://forum.arduino.cc/index.php?topic=258722.0)



