#ifndef UTIL_H
#define UTIL_H

#include <stdarg.h>

#define DEBUG 1
extern char buf[128];

#ifdef DEBUG
#define DEBUGP(...) do { serial_bprintf(buf, __VA_ARGS__); } while(false)
#else
#define DEBUGP(...) do{} while(false)
#endif

#define uint32Value(x, y) (x << 16 | y)
#define floatValue(x) (*(float*)x)
#define int8LsbValue(x) ((uint8_t)x)
#define int8MsbValue(x) ((uint8_t) x >> 8)
#define uint8LsbValue(x) int8LsbValue(x)
#define uint8MsbValue(x) int8MsbValue(x)

void serial_printf(const char *fmt, ...);
void serial_bprintf(char *buf, const char *fmt, ...);

#endif
