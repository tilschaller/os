#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <kstdio.h>
#include <string.h>

#include <ssfn.h>

static bool print(const char *data, size_t length)
{
    const unsigned char *bytes = (const unsigned char *)data;
    for (size_t i = 0; i < length; i++)
    {
        if (bytes[i] == '\n')
        {
            ssfn_dst.y += 20;
            ssfn_dst.x = 0;
        }
        if (ssfn_putc(bytes[i]) == EOF)
            return false;
    }
    return true;
}

int kprintf(const char *restrict format, ...)
{
    va_list parameters;
    va_start(parameters, format);

    int written = 0;

    while (*format != '\0')
    {
        size_t maxrem = INT_MAX - written;

        if (format[0] != '%' || format[1] == '%')
        {
            if (format[0] == '%')
                format++;
            size_t amount = 1;
            while (format[amount] && format[amount] != '%')
                amount++;
            if (maxrem < amount)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!print(format, amount))
                return -1;
            format += amount;
            written += amount;
            continue;
        }

        const char *format_begun_at = format++;

        if (*format == 'c')
        {
            format++;
            char c = (char)va_arg(parameters, int /* char promotes to int */);
            if (!maxrem)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!print(&c, sizeof(c)))
                return -1;
            written++;
        }
        else if (*format == 's')
        {
            format++;
            const char *str = va_arg(parameters, const char *);
            size_t len = strlen(str);
            if (maxrem < len)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!print(str, len))
                return -1;
            written += len;
        }
        else if (*format == 'd')
        {
            format++;
            int original = 0, reversed = 0, remainder = 0, size = 0;
            char c;
            original = va_arg(parameters, int);
            if (original == 0)
            {
                c = (char)0x30;
                print(&c, 1);
                goto printf_d_end;
            }
            while (original)
            {
                remainder = original % 10;
                reversed = reversed * 10 + remainder;
                original /= 10;
                size++;
            }
            for (original = 0; original < size; ++original)
            {
                c = (reversed % 10) + '0';
                print(&c, 1);
                reversed /= 10;
            }
        printf_d_end:
            size = 0;
            // TODO: fix printing integers
        }
        else
        {
            format = format_begun_at;
            size_t len = strlen(format);
            if (maxrem < len)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!print(format, len))
                return -1;
            written += len;
            format += len;
        }
    }

    va_end(parameters);
    return written;
}
