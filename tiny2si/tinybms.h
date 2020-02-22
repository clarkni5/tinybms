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

#define CELL_COUNT_REGISTER 53
#define PACK_VOLTAGE_REGISTER_0 36
#define PACK_CURRENT_REGISTER_0 38
#define MIN_CELL_VOLTAGE_REGISTER 40
#define MAX_CELL_VOLTAGE_REGISTER 41
#define MAX_DISCHARGE_CURRENT_REGISTER 102
#define MAX_CHARGE_CURRENT_REGISTER 103
#define PACK_CAPACITY_REGISTER 306
#define PACK_SOC_REGISTER_0 46
#define PACK_SOC2_REGISTER 328

extern AltSoftSerial *softSerial;
extern ModbusMaster *modbus;

typedef struct _battery_voltage {

	uint16_t cell_voltage[MAX_CELL_COUNT];

	union _pack_voltage {

		float fvoltage;
		uint16_t ivoltage[2];

	} pack_voltage;

	uint16_t min_cell_voltage;
	uint16_t max_cell_voltage;

	unsigned long last_success;

} Battery_voltage;

typedef struct _battery_current {

	float pack_current;
	uint16_t max_discharge_current;
	uint16_t max_charge_current;

	unsigned long last_success;

} Battery_current;

typedef struct _battery_config {

	uint16_t cell_count;
	uint16_t capacity;

	unsigned long last_success;

} Battery_config;

typedef struct _battery_soc {

	uint16_t stateOfCharge;
	uint16_t stateOfHealth;
	uint32_t stateOfChargeHp;

	unsigned long last_success;

} Battery_soc;

void init_tinybms();
void reset_tinybms();
int readRegistersWithRetry(uint16_t idx, uint16_t count, uint16_t *dest,
		uint8_t retrcnt);
int load_battery_voltage(Battery_config *config, Battery_voltage *voltage);
int load_battery_current(Battery_current *current);
int load_battery_config(Battery_config *config);
int load_battery_soc(Battery_soc *soc);
int load_battery_soc2(Battery_soc *soc);

#endif
