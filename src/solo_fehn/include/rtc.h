/*
 * rtc.h - CMOS Real-Time Clock driver
 *
 * The PC has a tiny battery-backed chip that holds the real wall-clock
 * time even when the machine is off. We read it through two I/O ports:
 *
 *   port 0x70  - select which CMOS register to access
 *   port 0x71  - read or write the value of the selected register
 *
 * The chip stores time in either BCD or binary format depending on a
 * status flag; rtc_read() handles either case and returns plain binary
 * values that the caller can use directly.
 */

#pragma once
#include <libc/stdint.h>

typedef struct {
    uint8_t  second;   /* 0..59 */
    uint8_t  minute;   /* 0..59 */
    uint8_t  hour;     /* 0..23 (always returned in 24-hour form) */
    uint8_t  day;      /* 1..31 */
    uint8_t  month;    /* 1..12 */
    uint16_t year;     /* full year, e.g. 2026 */
} rtc_time_t;

/* Read the current time from the CMOS RTC into *out. */
void rtc_read(rtc_time_t* out);
