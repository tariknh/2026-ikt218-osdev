#pragma once
#include <stdint.h>

/**
 * Read one byte from specified port
 * @param port port to read from
 * @return read byte
 */
unsigned char port_byte_in(uint16_t port) __attribute__((const));

/**
 * Write one byte to specified port
 * @param port  port to write to
 * @param data byte to write
 */
void port_byte_out(uint16_t port, uint8_t data);

/**
 * Read one word from specified port
 * @param port port to read from
 * @return read word
 */
unsigned short port_word_in(uint16_t port) __attribute__((const));

/**
 * Write one word to specified port
 * @param port port to write to
 * @param data word to write
 */
void port_word_out(uint16_t port, uint16_t data);
