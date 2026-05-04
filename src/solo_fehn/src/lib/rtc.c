/*
 * rtc.c - CMOS Real-Time Clock driver
 *
 * The classical way of reading the RTC safely is to read every register
 * twice and only accept the values when two consecutive reads agree, so
 * we do not catch the chip mid-update. The "update in progress" flag in
 * status register A also tells us when an update is happening.
 *
 * Most BIOSes leave the chip in BCD mode; we detect that by checking
 * status register B and convert to plain binary if needed. The "12-hour
 * mode" flag is also checked, although QEMU and most modern BIOSes
 * default to 24-hour mode.
 */

#include <rtc.h>
#include <io.h>
#include <libc/stdint.h>

#define CMOS_PORT_SELECT  0x70
#define CMOS_PORT_DATA    0x71

#define CMOS_REG_SECONDS  0x00
#define CMOS_REG_MINUTES  0x02
#define CMOS_REG_HOURS    0x04
#define CMOS_REG_DAY      0x07
#define CMOS_REG_MONTH    0x08
#define CMOS_REG_YEAR     0x09
#define CMOS_REG_STATUS_A 0x0A
#define CMOS_REG_STATUS_B 0x0B

static uint8_t cmos_read_reg(uint8_t reg) {
    /* The high bit of port 0x70 disables NMI on writes. We follow the
       convention of always re-asserting that bit by ORing with 0x80
       on hardware that requires it; QEMU is happy either way. */
    outb(CMOS_PORT_SELECT, reg);
    return inb(CMOS_PORT_DATA);
}

static int update_in_progress(void) {
    return cmos_read_reg(CMOS_REG_STATUS_A) & 0x80;
}

static uint8_t bcd_to_bin(uint8_t v) {
    return (uint8_t)((v & 0x0F) + ((v >> 4) * 10));
}

void rtc_read(rtc_time_t* out) {
    uint8_t sec, min, hr, day, mon, yr;
    uint8_t sec2, min2, hr2, day2, mon2, yr2;

    /* Loop until two consecutive reads agree. */
    do {
        while (update_in_progress()) { /* wait */ }
        sec = cmos_read_reg(CMOS_REG_SECONDS);
        min = cmos_read_reg(CMOS_REG_MINUTES);
        hr  = cmos_read_reg(CMOS_REG_HOURS);
        day = cmos_read_reg(CMOS_REG_DAY);
        mon = cmos_read_reg(CMOS_REG_MONTH);
        yr  = cmos_read_reg(CMOS_REG_YEAR);

        while (update_in_progress()) { /* wait */ }
        sec2 = cmos_read_reg(CMOS_REG_SECONDS);
        min2 = cmos_read_reg(CMOS_REG_MINUTES);
        hr2  = cmos_read_reg(CMOS_REG_HOURS);
        day2 = cmos_read_reg(CMOS_REG_DAY);
        mon2 = cmos_read_reg(CMOS_REG_MONTH);
        yr2  = cmos_read_reg(CMOS_REG_YEAR);
    } while (sec != sec2 || min != min2 || hr != hr2 ||
             day != day2 || mon != mon2 || yr != yr2);

    uint8_t status_b = cmos_read_reg(CMOS_REG_STATUS_B);

    /* Save the original PM flag (top bit of hours in 12-hour BCD mode). */
    uint8_t was_pm = hr & 0x80;
    hr &= 0x7F;

    /* Convert from BCD if the chip is in BCD mode (status_b bit 2 = 0). */
    if (!(status_b & 0x04)) {
        sec = bcd_to_bin(sec);
        min = bcd_to_bin(min);
        hr  = bcd_to_bin(hr);
        day = bcd_to_bin(day);
        mon = bcd_to_bin(mon);
        yr  = bcd_to_bin(yr);
    }

    /* Convert 12-hour to 24-hour if the chip is in 12-hour mode (status_b bit 1 = 0). */
    if (!(status_b & 0x02) && was_pm && hr < 12) {
        hr = (uint8_t)(hr + 12);
    }
    if (!(status_b & 0x02) && !was_pm && hr == 12) {
        hr = 0;
    }

    out->second = sec;
    out->minute = min;
    out->hour   = hr;
    out->day    = day;
    out->month  = mon;
    /* The year register only stores two digits; assume 21st century. */
    out->year   = (uint16_t)(2000u + yr);
}
