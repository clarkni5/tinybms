/**
 * ModbusMaster library: https://github.com/4-20ma/ModbusMaster
 *
 * Requires a JST 2.0mm 4-pin connector to connect to the TinyBMS.
 * Modbus communication is over RS232.
 *
 * AltSofteSerial modbus connections:
 * WHITE  > X      - not used (this is 5V coming from the TinyBMS)
 * YELLOW > pin  8 - data input pin (this is the TinyBMS Tx pin)
 * BLACK  > pin  9 - data output pin (this is the TinyBMS Rx pin)
 * RED    > ground - ground reference for I/O pins
 */

#ifndef TINYBMS_H
#define TINYBMS_H

#include <Arduino.h>
#include <ModbusMaster.h>
#include <AltSoftSerial.h>

#define MODBUS_RX_PIN 8
#define MODBUS_TX_PIN 9
#define MODBUS_BAUD 115200

#define TINYBMS_DEVICE_ID 0xAA
#define MODBUS_INTERVAL 100
#define MODBUS_RETRY_INTERVAL 100
#define MODBUS_RETRY_COUNT 10

#define MAX_CELL_COUNT 16

extern AltSoftSerial *softSerial;
extern ModbusMaster *modbus;

typedef struct _battery_voltage {

  uint16_t cell_count;

  union _cell_voltage {

    float fvoltage[MAX_CELL_COUNT];
    uint16_t ivoltage[MAX_CELL_COUNT*2];

  } cell_voltage;

  union _pack_voltage {

     float fvoltage;
     uint16_t ivoltage[2];

  } pack_voltage;

  unsigned long last_success;

} Battery_voltage;

void init_tinybms();
int readRegistersWithRetry(uint8_t idx, uint8_t count, uint16_t *dest, uint8_t retrcnt);
int load_battery_voltage(struct _battery_voltage *voltage);

#endif
