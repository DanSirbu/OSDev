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
			str[i] = rem + '0';
		} else {
			str[i] = rem + 0x57; //0x57 + 0xA = 'a'
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
char *strchr(const char *s, int c)
{
	size_t len = strlen(s) + 1; //include '\0' at end of the string
	for (size_t x = 0; x < len; x++) {
		if (s[x] == c) {
			return (char *)&s[x];
		}
	}
	return NULL;
}
char *strrchr(const char *s, int c)
{
	size_t len = strlen(s) + 1; //include '\0' at end of the string
	for (size_t x = len; x >= 0; x--) {
		if (s[x] == c) {
			return (char *)&s[x];
		}
	}
	return NULL;
}
bool contains(const char *input, char c)
{
	size_t len = strlen(input);
	for (size_t i = 0; i < len; i++) {
		if (input[i] == c) {
			return true;
		}
	}
	return false;
}
size_t strspn(const char *s, const char *accept)
{
	//TODO, there is a faster algo
	size_t len = strlen(s);

	size_t numOccurences = 0;
	for (size_t x = 0; x < len; x++) {
		if (!contains(accept, s[x])) {
			break;
		}
		numOccurences++;
	}
	return numOccurences;
}
size_t strcspn(const char *s, const char *reject)
{
	//TODO, there is a faster algo
	size_t len = strlen(s);

	size_t numNotContains = 0;
	for (size_t x = 0; x < len; x++) {
		if (contains(reject, s[x])) {
			break;
		}
		numNotContains++;
	}
	return numNotContains;
}

char *strtok(char *s, const char *delim)
{
	//NOTE: TAKEN FROM MUSL
	static char *strtok_ptr;
	if (!s && !(s = strtok_ptr))
		return NULL;

	s += strspn(s, delim);
	if (*s == '\0')
		return strtok_ptr = 0;
	strtok_ptr = s + strcspn(s, delim);
	if (*strtok_ptr)
		*strtok_ptr++ = 0;
	else
		strtok_ptr = 0;

	return s;
}
/*
Returns the basename of the path
*/
char *getBasename(const char *s)
{
	char *prevSlashNextIndex = (char *)s;
	for (size_t x = 0; x < strlen(s); x++) {
		if (x > 0 && s[x - 1] == '\\') {
			continue;
		}
		if (s[x] == '/') {
			prevSlashNextIndex = (char *)&s[x] + 1;
		}
	}

	return prevSlashNextIndex;
}
/*
 * Note: This modifies the string so make sure to pass a copy
 */
char *getFilenameNoExt(char *s)
{
	char *basename = getBasename(s);
	return strtok(basename, ".");
}

char *strsep(char **stringp, const char *delim)
{
	if (*stringp == NULL || **stringp == (int)NULL) {
		return NULL;
	}
	char *ret = *stringp;

	char *cur = *stringp;
	uint8_t numDelim = strlen(delim);
	while (true) {
		if (*cur == '\0') {
			break;
		}
		for (uint8_t i = 0; i < numDelim; i++) {
			if (*cur == delim[i]) {
				goto strsep_break;
			}
		}
		cur++;
	}
strsep_break:
	if (*cur == '\0') {
		*stringp = NULL;
		return ret;
	}
	*cur = '\0';
	*stringp = cur + 1;

	return ret;
}

char **split(char *str, char *delim)
{
	char *strCopy = strdup(str);

	char *strCopy2 = strCopy;
	char **strSepCur = &strCopy2;

	char *token;
	int numTokens = 0;
	while ((token = strsep(strSepCur, delim)) != NULL) {
		numTokens++;
	}

	strcpy(strCopy, str);

	strCopy2 = strCopy;
	strSepCur = &strCopy2;
	char **arr = malloc(sizeof(char *) *
			    (numTokens + 1)); //+ 1 for null terminator token
	int i = 0;
	while ((token = strsep(strSepCur, delim)) != NULL) {
		arr[i] = strdup(token);
		i++;
	}
	arr[i] = NULL;

	free(strCopy);
	return arr;
}
char *strcat(char *dest, const char *src)
{
	char *destAddr = (char *)dest;
	while (*destAddr != '\0') {
		destAddr++;
	}
	char *srcAddr = (char *)src;
	while (*srcAddr != '\0') {
		*destAddr = *srcAddr;
		destAddr++;
		srcAddr++;
	}
	*destAddr = '\0';

	return dest;
}
char *rindex(const char *s, int c)
{
	size_t len = strlen(s);
	char *lastOccurence = NULL;
	for (size_t x = 0; x < len; x++) {
		if (s[x] == c) {
			lastOccurence = (char *)&s[x];
		}
	}

	return lastOccurence;
}