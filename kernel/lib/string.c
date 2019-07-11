#include "string.h"
#include "kmalloc.h"

size_t strlen(const char *str)
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

void itoa(uint32_t number, char *str, uint32_t base)
{
	if (number == 0) {
		str[0] = '0';
		str[1] = '\0';
		return;
	}
	uint32_t i = 0;
	uint32_t lowBytes = number; //& 0xFFFFFFFF;
	while (lowBytes != 0) {
		uint32_t rem = lowBytes % base;
		if (rem < 10) {
			str[i] = rem + ASCII_NUMBER_CONST;
		} else {
			str[i] = rem + ASCII_LETTER_CONST;
		}
		lowBytes /= base;
		i++;
	}
	/*uint32_t highBytes = (number >> 31) >> 1 & 0xFFFFFFFF; //>> 32, but IA-32
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
char *strcpy(char *dst, char *src)
{
	uint32_t len = strlen(src);
	uint32_t x;
	for (x = 0; x < len; x++) {
		dst[x] = src[x];
	}
	dst[x] = '\0';

	return dst;
}
char *strconcat(char *dst, char *str1, char *str2)
{
	strcpy(dst, str1);
	strcpy(dst + strlen(str1), str2);
	return dst;
}
char *strncpy(char *dest, const char *src, size_t n)
{
	size_t x;
	for (x = 0; x < n; x++) {
		dest[x] = src[x];
		if (src[x] == '\0') {
			break;
		}
	}
	for (; x < n; x++) {
		dest[x] = '\0';
	}

	return dest;
}
int strncmp(char *str1, char *str2, size_t max_len)
{
	int n = max_len;
	char *p = str1;
	char *q = str2;

	while (n > 0 && *p && *p == *q) {
		n--;
		p++;
		q++;
	}
	if (n == 0) {
		return 0;
	}
	return (int)(*p - *q);
}

int strcmp(const char *str1, const char *str2)
{
	char *p = (char *)str1;
	char *q = (char *)str2;

	while (*p && *p == *q) {
		p++;
		q++;
	}
	return (int)(*p - *q);
}

size_t array_length(char *arr[])
{
	size_t i = 0;
	while (arr[i] != NULL) {
		i++;
	}
	return i;
}
char *strdup(const char *s)
{
	size_t len = strlen(s) + 1;

	char *new_str = kmalloc(len);
	memcpy(new_str, s, len);

	return new_str;
}