#include "../include/string.h"

u32 strlen(char *str) {
	int i = 0;
	while(str[i] != '\0') { i++; }
	return i;
}
void reverse(char *str) {
	char *start = str;
	char *end = str + strlen(str) - 1;
	char temp;

	while(end > start) {
		temp = *start;
		*start = *end;
		*end = temp;

		start++;
		end--;
	}
}

void itoa(u32 number, char *str, u32 base) {
	if(number == 0) {
		str[0] = '0';
		str[1] = '\0';
		return;
	}
	u32 i = 0;
	u32 lowBytes = number; //& 0xFFFFFFFF;
	while(lowBytes != 0) {
		u32 rem = lowBytes % base;
		if(rem < 10) {
			str[i] = rem + ASCII_NUMBER_CONST;
		} else {
			str[i] = rem + ASCII_LETTER_CONST;
		}
		lowBytes /= base;
		i++;
	}
	/*u32 highBytes = (number >> 31) >> 1 & 0xFFFFFFFF; //>> 32, but IA-32 processors mask shift to 5 bits so max is shift is 31
	while(highBytes != 0) {
		u64 rem = highBytes % base;
		if(rem < 10) {
			str[i] = rem + 0x30;
		} else {
			str[i] = rem + 0x57;
		}
		highBytes /= base;
		i++;
	}*/
	str[i] = '\0';

	reverse(str);
}