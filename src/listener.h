#include <Arduino.h>
#include <common.h>


/**
 * @brief Function to handle interrupt triggered by the listener.
 */
void listener_interrupt();

/**
 * @brief Function to set up the listener.
 */
void listenerSetup();

/**
 * @brief Function to continuously loop and process listener events.
 * @return True if the listener loop is successful, false otherwise.
 */
bool listenerLoop();

/**
 * @brief Pointer to a function that is called when an interrupt is triggered.
 * @param buffer A pointer to a volatile byte buffer.
 */
extern void (*onBuffer)(volatile byte *buffer);