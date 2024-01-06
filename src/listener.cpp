#include "listener.h"



volatile unsigned long lastRisingEdge = 0;
volatile unsigned long highDuration = 0;
volatile bool startOfCapture = false;

volatile byte buffer[20]= {0}; // This is where the bits are stored
volatile byte buffer_index = 0;
volatile byte bitCount = 0;

void (*onBuffer)(volatile byte *buffer);


/**
 * @brief Interrupt service routine for capturing data from a sensor.
 * 
 * This function is called when an interrupt is triggered. It captures data from a sensor
 * connected to the PIN_NET pin. The captured data is stored in a buffer for further processing.
 * 
 * @note This function assumes that the PIN_NET pin is configured as an input pin.
 * 
 * @note This function should be called within an interrupt context.
 */
void listener_interrupt() {
    unsigned long currentTime = micros();
    if (digitalRead(PIN_NET) == HIGH) { // Rising edge
        lastRisingEdge = currentTime;
    } else { // Falling edge
        highDuration = currentTime - lastRisingEdge;
        if (highDuration >= 4000 && highDuration <= 5640) { // 4.7ms high is start of capture, allow 20% deviation
            startOfCapture = true;
            buffer_index = 0;
            bitCount = 0;
            buffer[0] = 0; // Reset the first byte
        } else if (startOfCapture) { // first bit is LSB
            if (highDuration >= 2560 && highDuration <= 3840) { // 3.2ms high is a 0, allow 20% deviation
                // Do nothing, 0 is already on the right
            } else if (highDuration >= 920 && highDuration <= 1380) { // 1.15ms high is a 1, allow 20% deviation
                buffer[buffer_index] |= (1 << bitCount); // Set the bit to 1
            }
            else {
                log_e("Invalid bit duration: %d", highDuration);
                // Invalid bit, ignore
                return;
            }
            bitCount++;
            if (bitCount % 8 == 0 && buffer_index < sizeof(buffer) - 1) { // Start of a new byte
                buffer_index++;
                buffer[buffer_index] = 0;
                bitCount = 0;
            }
            else if (buffer_index >= sizeof(buffer) - 1) {
                log_e("Buffer overflow!");//most likely a frame collision
                startOfCapture = false;
                buffer_index = 0;
                bitCount = 0;
                memset((void *) buffer, 0, sizeof(buffer));
                return;
            }
        }
    }
}

void listenerSetup(){
    pinMode(PIN_NET, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_NET), listener_interrupt, CHANGE);
}


/**
 * @brief This function runs a loop to listen for data incoming from the interrupt service routine listener_interrupt().
 * 
 * @return true if a complete buffer is received, false otherwise.
 */
bool listenerLoop(){// return true if we got a complete buffer
    // if the last rising edge was more than 50ms, we can assume the transmission is over
    if (startOfCapture && (micros() - lastRisingEdge) > 100000) {
        startOfCapture = false;
        // Handle buffer here
        if (buffer_index == 10){
            log_i("Got 80 bits, calling onBuffer.");
            //call onBuffer function, pause interrupts while we do it
            detachInterrupt(digitalPinToInterrupt(PIN_NET));
            if (onBuffer != NULL)
                onBuffer(buffer);
            else
                log_e("onBuffer is NULL!");
            attachInterrupt(digitalPinToInterrupt(PIN_NET), listener_interrupt, CHANGE);
        } else {
            Serial.print("Got ");
            Serial.print(buffer_index);
            Serial.println(" bits, ignoring.");
            //display buffer in binary
            for (byte i = 0; i < buffer_index; i++){
                Serial.print(buffer[i], BIN);
                Serial.print(" ");
            }

        }
    }
    return false;
}