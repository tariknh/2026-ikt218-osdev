#pragma once
#include <stdint.h>


/**
 * Initialize the PCSPK, just stops any currently playing sound since it doesnt require any specific
 * initialization
 */
void PCSPK_init();
/**
 * Play a square wave with the given frequency
 * @param frequency
 */
void PCSPK_PLAY(uint32_t frequency);
/**
 * Stop the currently playing tone. If no tone is playing, nothing happens.
 */
void PCSPK_STOP();
