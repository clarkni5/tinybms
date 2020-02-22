#include <Arduino.h>
#include "util.h"

void serial_bprintf(char *buf, const char *fmt, ...) {
	va_list argp;
	va_start(argp, fmt);
	vsprintf(buf, fmt, argp);
	Serial.print(buf);
	va_end(argp);
}

void serial_printf(const char *fmt, ...) {
	char buf[128];

	va_list argp;
	va_start(argp, fmt);
	vsprintf(buf, fmt, argp);
	Serial.print(buf);
	va_end(argp);
}

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
	char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
	return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}
