#include "string.h"

uint32_t strlen(char *str)
{
	int i = 0;
	while (str[i] != '\0') {
		i++;
	}
	return i;
}
void reverse(char *str)
{
	char *start = str;
	char *end = str + strlen(str) - 1;
	char temp;

	while (end > start) {
		temp = *start;
		*start = *end;
		*end = temp;

		start++;
		end--;
	}
}

void itoa(u32 number, char *str, u32 base)
{
	if (number == 0) {
		str[0] = '0';
		str[1] = '\0';
		return;
	}
	u32 i = 0;
	u32 lowBytes = number; //& 0xFFFFFFFF;
	while (lowBytes != 0) {
		u32 rem = lowBytes % base;
		if (rem < 10) {
			str[i] = rem + ASCII_NUMBER_CONST;
		} else {
			str[i] = rem + ASCII_LETTER_CONST;
		}
		lowBytes /= base;
		i++;
	}
	/*u32 highBytes = (number >> 31) >> 1 & 0xFFFFFFFF; //>> 32, but IA-32
  processors mask shift to 5 bits so max is shift is 31 while(highBytes != 0) {
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
void strcpy_max_len(char *src, char *dest, uint32_t maxLen)
{
	uint32_t x;
	for (x = 0; x < maxLen; x++) {
		dest[x] = src[x];
	}
	dest[x] = '\0';
}
void strcpy(char *dst, char *src)
{
	uint32_t len = strlen(src);
	uint32_t x;
	for (x = 0; x < len; x++) {
		dst[x] = src[x];
	}
	dst[x] = '\0';
}
int strncmp(char *str1, char *str2, size_t max_len) {
	int n = max_len;
	char *p;
	char *q;

	while(n > 0 && *p && *p == *q) {
		n--;
		p++;
		q++;
	}
	if(n == 0) {
		return 0;
	}
	return (int) (*p - *q);
}

int strcmp(char *str1, char *str2) {
	uint32_t str1_len = strlen(str1);
	uint32_t str2_len = strlen(str2);

	if(str1_len != str2_len) {
		return 0;
	}
	for(uint32_t x = 0; x < str1_len; x++) {
		if(str1[x] != str2[x]) {
			return 0;
		}
	}

	return 1;
}