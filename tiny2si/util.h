#ifndef UTIL_H
#define UTIL_H

#include <stdarg.h>
#include <Arduino.h>

#undef DEBUG
//#define DEBUG 1
extern char buf[128];

#ifdef DEBUG
#define DEBUGP(...) do { serial_bprintf(buf, __VA_ARGS__); } while(false)
#else
#define DEBUGP(...) do{} while(false)
#endif

#define INT16_MIN -32768
#define FLOAT_MIN -3.4028235E+38
#define FLOAT_MAX 3.4028235E+38

#define uint32Value(x, y) (x << 16 | y)
#define floatValue(x) (*(float*)x)
#define int8LsbValue(x) ((uint8_t)x)
#define int8MsbValue(x) ((uint8_t) x >> 8)
#define uint8LsbValue(x) int8LsbValue(x)
#define uint8MsbValue(x) int8MsbValue(x)

void serial_printf(const char *fmt, ...);
void serial_bprintf(char *buf, const char *fmt, ...);
void serial_sbprintf(HardwareSerial *serial, char *buf, const char *fmt, ...);
int get_free_memory();

// Arduino is LE, so this is a noop
#define leWord(x) (x)

uint8_t* array_dup(uint8_t *src, uint16_t size);

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#endif
