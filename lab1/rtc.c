#include "rtc.h"
#include <minix/syslib.h>
#include <minix/sysutil.h>

#define TODO return -1

#define RTC_ADDR_REG 0x70
#define RTC_DATA_REG 0x71
#define RTC_REG_A 0x0A
#define RTC_REG_B 0x0B
#define RTC_REG_DAY 0x07
#define RTC_REG_MONTH 0x08
#define RTC_REG_YEAR 0x09
#define RTC_UIP_MSK (1 << 7)
#define RTC_DM_MSK (1 << 2)

static int bcd_to_bin(uint8_t bcd) { 
  return ((bcd >> 4) * 10) + (bcd & 0x0F); }

int rtc_read_date(rtc_date *date) { 
  if (date == NULL) return 1;
  
  uint32_t regA = 0, regB = 0;
  uint32_t day = 0, month = 0, year = 0;

  if (sys_outb(RTC_ADDR_REG, RTC_REG_B) != 0) return 1;
  if (sys_inb(RTC_DATA_REG, &regB) != 0) return 1;

  int is_bcd = !(regB & RTC_DM_MSK);

  do {
    if (sys_outb(RTC_ADDR_REG, RTC_REG_A) != 0) return 1;
    if (sys_inb(RTC_DATA_REG, &regA) != 0) return 1;

    if (regA & RTC_UIP_MSK) {
      tickdelay(micros_to_ticks(20000));
    }
  } while (regA & RTC_UIP_MSK);

  if (sys_outb(RTC_ADDR_REG, RTC_REG_DAY) != 0) return 1;
  if (sys_inb(RTC_DATA_REG, &day) != 0) return 1;

  if (sys_outb(RTC_ADDR_REG, RTC_REG_MONTH) != 0) return 1;
  if (sys_inb(RTC_DATA_REG, &month) != 0) return 1;

  if (sys_outb(RTC_ADDR_REG, RTC_REG_YEAR) != 0) return 1;
  if (sys_inb(RTC_DATA_REG, &year) != 0) return 1;

  if (is_bcd) {
    date->day = bcd_to_bin((uint8_t)day);
    date->month = bcd_to_bin((uint8_t)month);
    date->year = bcd_to_bin((uint8_t)year);
  } else {
    date->day = (uint8_t)day;
    date->month = (uint8_t)month;
    date->year = (uint8_t)year;
  }

  return 0;
}
