#ifndef SUNNYISLAND_H
#define SUNNYISLAND_H

#include <Arduino.h>
#include <SPI.h>
#include <mcp_can.h>
#include "util.h"
#include "tinybms.h"

#define loHi(x,y) ((uint16_t)(y << 8 | x))
// Arduino is LE, so this is a noop
#define leWord(x) (x)

#define CAN_CS_PIN 53
#define CAN_DELAY 210

#define SI_CHARGE_PARAMS_FRAME 0x351
#define SI_VOLTAGE_FRAME 0x356
#define SI_SOC_FRAME 0x355
#define SI_ID_FRAME 0x35F
#define SI_NAME_FRAME 0x35E
#define SI_FAULT_FRAME 0x35A

#define BMS_NAME "TinyBMS"

// General (F920 XA01 General)
#define FAULT0_GENERAL 0b00000001
// Battery overvoltage (F921 XA02 DcHiVolt)
#define FAULT0_OVERVOLTAGE 0b00000100
// Battery undervoltage (F922 XA03 DcLoVolt)
#define FAULT0_UNDERVOLTAGE 0b00010000
// Battery overtemperature (F923 XA04 DcHiTmp)
#define FAULT0_OVERTEMP 0b01000000
// Battery undertemperature (F924 XA05 DcLoTmp)
#define FAULT1_UNDERTEMP 0b00000001
// Battery overtemperature in charge mode (F925 XA06 DcHiTmpC)
#define FAULT1_OVERTEMP 0b00000100
// Battery undertemperature in charge mode (F926 XA07 DcLoTmpC)
#define FAULT1_CHARGE_UNDERTEMP 0b00010000
// Battery discharge current too high (F927 XA08 DcHiCur)
#define FAULT1_DISCHARGE_OVERCURRENT 0b01000000
// Battery charging current too high (F928 XA09 DcHiChgCur)
#define FAULT2_CHARGE_OVERCURRENT 0b00000001
// Contactor (F929 XA10 Contact)
#define FAULT2_CONTACTOR 0b00000100
// Short circuit (F930 XA11 Short)
#define FAULT2_SHORTCIRC 0b00010000
// Internal error in battery management (F931 XA12 BMS)
#define FAULT2_BMS_ERROR 0b01000000
// Different state of charge of the battery cells (F932 XA13 CellBal)
#define FAULT3_BATTERY_DISBALANCE 0b00000001
// Not used (F933 XA14)
#define FAULT3_F933 0b00000100
// Not used (F934 XA15)
#define FAULT3_F934 0b00010000
// Error in the generator (F935 XA16 Generator)
#define FAULT3_GENERATOR_ERROR 0b01000000

extern MCP_CAN *CAN;

uint8_t* make_charge_params_frame(uint16_t finalChargeVoltage,
		uint16_t maxChargeCurrent, uint16_t maxDischargeCurrent,
		uint16_t finalDischargeVoltage);
void parse_charge_params_frame(uint8_t frame[], uint16_t *finalChargeVoltage,
		uint16_t *maxChargeCurrent, uint16_t *maxDischargeCurrent,
		uint16_t *finalDischargeVoltage);
uint8_t* make_soc_frame(uint16_t stateOfCharge, uint16_t stateOfHealth,
		uint16_t stateOfChargeHighPrecision);
void parse_soc_frame(uint8_t frame[], uint16_t *stateOfCharge,
		uint16_t *stateOfHealth, uint16_t *stateOfChargeHighPrecision);
uint8_t* make_voltage_frame(uint16_t batteryVoltage, uint16_t batteryCurrent,
		uint16_t batteryTemp);
void parse_voltage_frame(uint8_t frame[], uint16_t *batteryVoltage,
		uint16_t *batteryCurrent, uint16_t *batteryTemp);
uint8_t* make_charge_params_frame2(uint16_t finalChargeVoltage,
		uint16_t maxChargeCurrent, uint16_t maxDischargeCurrent,
		uint16_t finalDischargeVoltage);
uint8_t* make_charge_params_frame3(uint16_t finalChargeVoltage,
		uint16_t maxChargeCurrent, uint16_t maxDischargeCurrent,
		uint16_t finalDischargeVoltage);

void init_sunnyisland();
void send_voltage_frame(Battery_voltage *voltage);
void send_name_frame();
void send_id_frame(uint16_t batteryCapacity);
void send_soc_frame(Battery_soc *soc);
void send_fault_frame(uint8_t f0, uint8_t f1, uint8_t f2, uint8_t f3);

#endif

