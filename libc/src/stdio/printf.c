#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

char buf[32];

static bool print(const char* data, size_t length) {
    const unsigned char* bytes = (const unsigned char*) data;
    for (size_t i = 0; i < length; i++)
        if (putchar(bytes[i]) == EOF)
            return false;
    return true;
}

int printf(const char* restrict format, ...) {
    va_list parameters;
    va_start(parameters, format);

    int written = 0;

    while (*format != '\0') {
        size_t maxrem = INT_MAX - written;

        if (format[0] != '%' || format[1] == '%') {
            if (format[0] == '%')
                format++;
            size_t amount = 1;
            while (format[amount] && format[amount] != '%')
                amount++;
            if (maxrem < amount) {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!print(format, amount))
                return -1;
            format += amount;
            written += amount;
            continue;
        }

        const char* format_begun_at = format++;

        if (*format == 'c') {
            format++;
            char c = (char) va_arg(parameters, int /* char promotes to int */);
            if (!maxrem) {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!print(&c, sizeof(c)))
                return -1;
            written++;
        } else if (*format == 's') {
            format++;
            const char* str = va_arg(parameters, const char*);
            size_t len = strlen(str);
            if (maxrem < len) {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!print(str, len))
                return -1;
            written += len;
        } else if(*format == 'd'){
            format++;
            int i = (int) va_arg(parameters, int /* char promotes to int */);
            if(i < 0) {
                putchar('-');
                written++;
            }
            if (!maxrem) {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            int j = 0;
            while (i != 0){
                buf[j] = (i % 10) + '0';
                i /= 10;
                j++;
            }
            written += j;
            while (j > 0){
                putchar(buf[j - 1]);
                j--;
            }
            memset(buf,0,32);
        }else if (*format == 'l'){
            format++;
            long i = (long) va_arg(parameters, long /* char promotes to long */);
            if(i < 0) {
                putchar('-');
                written++;
            }
            if (!maxrem) {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            int j = 0;
            while (i != 0){
                buf[j] = (i % 10) + '0';
                i /= 10;
                j++;
            }
            written += j;
            while (j > 0){
                putchar(buf[j - 1]);
                j--;
            }
            memset(buf,0,32);
        }else if (*format == 'm'){
            format++;
            unsigned long i = (unsigned long) va_arg(parameters, unsigned long /* char promotes to long */);
            if (!maxrem) {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            int j = 0;
            while (i != 0){
                char val = i % 16;
                if (val < 10) val += '0';
                else val += 'A' - 10;
                buf[j] = val;
                i /= 16;
                j++;
            }
            written += j;
            putchar('0');
            putchar('x');
            while (j > 0){
                putchar(buf[j - 1]);
                j--;
            }
            memset(buf,0,32);
        }else {
            format = format_begun_at;
            size_t len = strlen(format);
            if (maxrem < len) {
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