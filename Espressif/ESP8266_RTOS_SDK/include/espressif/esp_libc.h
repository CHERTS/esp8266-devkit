/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __ESP_LIBC_H__
#define __ESP_LIBC_H__

#ifdef __cplusplus
extern "C" {
#endif

char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
size_t strlen(const char *s);
char *strstr(const char *s1, const char *s2);
char *strcat(char *dst, const char *src);
char *strncat(char *dst, const char *src, size_t count);
size_t strspn(const char *s, const char *accept);
size_t strcspn(const char *s, const char *reject);
char *strtok_r(char *s, const char *delim, char **ptrptr);
char *strtok(char *s, const char *delim);
char *strrchr(const char *s, int c);
char *strdup(const char *s);
char *strchr(const char *s, int c);
long strtol(const char *str, char **endptr, int base);

void bzero(void *s, size_t n);

void *memcpy(void *dst, const void *src, size_t n);
void *memset(void *dst, int c, size_t n);
int memcmp(const void *m1, const void *m2, size_t n);
void *memmove(void *dst, const void *src, size_t n);

int rand(void);

int printf(const char *format, ...);
int sprintf(char *out, const char *format, ...);
int snprintf(char *buf, unsigned int count, const char *format, ...);
int puts(const char *str);

void *malloc(size_t n);
void free(void *p);
void *calloc(size_t c, size_t n);
void *zalloc(size_t n);
void *realloc(void *p, size_t n);

int atoi(const char *s);
long atol(const char *s);

unsigned long os_random(void);
int os_get_random(unsigned char *buf, size_t len);

/* NOTE: don't use printf_opt in irq handler, for test */
#define os_printf(fmt, ...) do {    \
        static const char flash_str[] ICACHE_RODATA_ATTR STORE_ATTR = fmt;  \
        printf(flash_str, ##__VA_ARGS__);   \
    } while(0)

#ifdef __cplusplus
}
#endif

#endif /* __LIBC_H__ */
