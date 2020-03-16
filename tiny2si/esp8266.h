#ifndef ESP8266_H
#define ESP8266_H

#include <Arduino.h>

extern HardwareSerial Serial3; // D15 RX, D14 TX
#define wifiSerial (&Serial3)

#define WIFI_BAUD 115200

int init_wifi();
int wifi_send_data(uint8_t *data, int len);

#endif
