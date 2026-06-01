#include <stddef.h>

void *memcpy(void *dest, const void *src, size_t n)
{
    unsigned char *d = dest;
    const unsigned char *s = src;

    while (n--)
        *d++ = *s++;

    return dest;
}

void *memset(void *dest, int c, size_t n)
{
    unsigned char *d = dest;

    while (n--)
        *d++ = (unsigned char)c;

    return dest;
}

/*
 * Basic stubs for libc functions that are sometimes pulled in by
 * application code when building freestanding/embedded projects.
 *
 * In this simple example we only need printf, which is used by the
 * demo task.  The implementation provided here is a minimal stub that
 * simply consumes its arguments and returns success.  In a real system
 * you would either link a more complete C library or implement a
 * backend that sends characters to a serial port, semihosting, etc.
 */

/* include stdarg once; real printf implementation follows below */
#include <stdarg.h>

/*
 * Minimal UART output for the PL011 peripheral.  The guest environment
 * provided by uvmm maps the virtual console at 0x3100_0000; the simple
 * driver below busy-waits on the transmit FIFO empty flag and writes
 * characters to the data register.  This allows us to see text on the
 * host's console without pulling in any C library support.
 */

#define UART0_BASE    0x31000000UL /* virtual PL011 console in uvmm */
#define UART_DR       (*(volatile unsigned int *)(UART0_BASE + 0x00))
#define UART_FR       (*(volatile unsigned int *)(UART0_BASE + 0x18))
#define UART_FR_TXFF  (1U << 5)

static void uart_putc(char c)
{
    /* wait until there is room in the FIFO */
    while (UART_FR & UART_FR_TXFF) {
        /* nop */
    }
    UART_DR = (unsigned int)c;
}

/*
 * Very small printf implementation that understands %s, %u, %llu and %%.
 * It formats into a local buffer and then emits each character via
 * uart_putc().  That's enough for the two calls in main.c.
 */

int printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char buf[128];
    char *p = buf;

    while (*fmt && (p - buf) < (int)sizeof(buf) - 1) {
        if (*fmt != '%') {
            *p++ = *fmt++;
            continue;
        }
        fmt++;
        switch (*fmt) {
        case 's': {
            const char *s = va_arg(ap, const char *);
            while (*s && (p - buf) < (int)sizeof(buf) - 1)
                *p++ = *s++;
            break;
        }
        case 'u': {
            unsigned long long v = va_arg(ap, unsigned long long);
            char num[32];
            int n = 0;
            if (v == 0)
                num[n++] = '0';
            else {
                while (v && n < (int)sizeof(num)) {
                    num[n++] = '0' + (v % 10);
                    v /= 10;
                }
            }
            while (n-- && (p - buf) < (int)sizeof(buf) - 1)
                *p++ = num[n];
            break;
        }
        case '%':
            *p++ = '%';
            break;
        default:
            /* unknown specifier, just output it */
            *p++ = *fmt;
            break;
        }
        fmt++;
    }
    *p = '\0';
    va_end(ap);

    for (p = buf; *p; ++p)
        uart_putc(*p);
    return 0;
}


