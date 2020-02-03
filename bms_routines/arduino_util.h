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
    
#ifdef __cplusplus
}
#endif

#endif /* ARDUINO_UTIL_H */

