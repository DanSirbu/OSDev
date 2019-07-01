//From osdev wiki
#include "cmos.h"

#define CENTURY 2000

int century_register = 0x00; // Set by ACPI table parsing code if possible

unsigned char second;
unsigned char minute;
unsigned char hour;
unsigned char day;
unsigned char month;
unsigned int year;

enum { cmos_address = 0x70, cmos_data = 0x71 };

int get_update_in_progress_flag()
{
	outb(cmos_address, 0x0A);
	return (inb(cmos_data) & 0x80);
}

unsigned char get_RTC_register(int reg)
{
	outb(cmos_address, reg);
	return inb(cmos_data);
}

//From toaruos, get number of seconds in year
uint32_t secs_of_years(int years)
{
	uint32_t days = 0;
	while (years > 1969) {
		days += 365;
		if (years % 4 == 0) {
			if (years % 100 == 0) {
				if (years % 400 == 0) {
					days++;
				}
			} else {
				days++;
			}
		}
		years--;
	}
	return days * 86400;
}
uint32_t secs_of_month(int months, int year)
{
	uint32_t days = 0;
	switch (months) {
	case 11:
		days += 30;
	case 10:
		days += 31;
	case 9:
		days += 30;
	case 8:
		days += 31;
	case 7:
		days += 31;
	case 6:
		days += 30;
	case 5:
		days += 31;
	case 4:
		days += 30;
	case 3:
		days += 31;
	case 2:
		days += 28;
		if ((year % 4 == 0) &&
		    ((year % 100 != 0) || (year % 400 == 0))) {
			days++;
		}
	case 1:
		days += 31;
	default:
		break;
	}
	return days * 86400;
}

uint32_t read_rtc_sec_from_epoch()
{
	unsigned char century;
	unsigned char last_second;
	unsigned char last_minute;
	unsigned char last_hour;
	unsigned char last_day;
	unsigned char last_month;
	unsigned char last_year;
	unsigned char last_century;
	unsigned char registerB;

	// Note: This uses the "read registers until you get the same values twice in a row" technique
	//       to avoid getting dodgy/inconsistent values due to RTC updates

	while (get_update_in_progress_flag())
		; // Make sure an update isn't in progress
	second = get_RTC_register(0x00);
	minute = get_RTC_register(0x02);
	hour = get_RTC_register(0x04);
	day = get_RTC_register(0x07);
	month = get_RTC_register(0x08);
	year = get_RTC_register(0x09);
	if (century_register != 0) {
		century = get_RTC_register(century_register);
	} else {
		century = CENTURY;
	}

	do {
		last_second = second;
		last_minute = minute;
		last_hour = hour;
		last_day = day;
		last_month = month;
		last_year = year;
		last_century = century;

		while (get_update_in_progress_flag())
			; // Make sure an update isn't in progress
		second = get_RTC_register(0x00);
		minute = get_RTC_register(0x02);
		hour = get_RTC_register(0x04);
		day = get_RTC_register(0x07);
		month = get_RTC_register(0x08);
		year = get_RTC_register(0x09);
		if (century_register != 0) {
			century = get_RTC_register(century_register);
		}
	} while ((last_second != second) || (last_minute != minute) ||
		 (last_hour != hour) || (last_day != day) ||
		 (last_month != month) || (last_year != year) ||
		 (last_century != century));

	registerB = get_RTC_register(0x0B);

	// Convert BCD to binary values if necessary

	if (!(registerB & 0x04)) {
		second = (second & 0x0F) + ((second / 16) * 10);
		minute = (minute & 0x0F) + ((minute / 16) * 10);
		hour = ((hour & 0x0F) + (((hour & 0x70) / 16) * 10)) |
		       (hour & 0x80);
		day = (day & 0x0F) + ((day / 16) * 10);
		month = (month & 0x0F) + ((month / 16) * 10);
		year = (year & 0x0F) + ((year / 16) * 10);
		if (century_register != 0) {
			century = (century & 0x0F) + ((century / 16) * 10);
		}
	}

	// Convert 12 hour clock to 24 hour clock if necessary

	if (!(registerB & 0x02) && (hour & 0x80)) {
		hour = ((hour & 0x7F) + 12) % 24;
	}

	// Calculate the full (4-digit) year

	if (century_register != 0) {
		year += century * 100;
	} else {
		year += CENTURY;
	}

	uint32_t time = secs_of_years(year) + secs_of_month(month, year) +
			day * (24 * 60 * 60) + hour * (60 * 60) + minute * 60 +
			second;

	return time;
}