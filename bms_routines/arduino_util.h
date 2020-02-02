#ifndef ARDUINO_UTIL_H
#define ARDUINO_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#define lowByte(x) ((unsigned char)(x & 0xff))
#define highByte(x) ((unsigned char)(x >> 8 & 0xff))
#define loHi(x,y) ((unsigned long)(y << 8 | x))

#ifdef __cplusplus
}
#endif

#endif /* ARDUINO_UTIL_H */

