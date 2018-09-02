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
	int curNumber = number;
	if(number == 0) {
		str[0] = '0';
		str[1] = '\0';
		return;
	}
	int i = 0;
	while(curNumber != 0) {
		int rem = curNumber % base;
		if(rem < 10) {
		str[i++] = rem + 0x30;
		} else {
			str[i++] = rem + 0x57;
		}
		curNumber /= base;
	}
	str[++i] = '\0';

	reverse(str);
}