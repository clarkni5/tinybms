#ifndef ARDUINO_UTIL_H
#define ARDUINO_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef short word;
typedef unsigned char byte;
 
#define lowByte(x) ((word)(x & 0xff))
#define highByte(x) ((word)(x >> 8 & 0xff))
#define loHi(x,y) ((word)(y << 8 | x))
#define leWord(x) ((word)((x & 0xff << 8) | (x & 0xff00 >> 8)))
    
#ifdef __cplusplus
}
#endif

#endif /* ARDUINO_UTIL_H */

