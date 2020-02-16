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
