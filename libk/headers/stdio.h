#ifndef __STDIO_H__
#define __STDIO_H__

#include <stdint.h>

#define EOF (-1)

int printk(const char *format, ...);
int putchar(char ch);
int puts(const char *string);

#endif // __STDIO_H__
