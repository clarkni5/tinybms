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

#define CAN_PIN 7
#define CAN_DELAY 210

#define SI_CHARGE_PARAMS_FRAME 0x351
#define SI_VOLTAGE_FRAME 0x356
#define SI_SOC_FRAME 0x355
#define SI_ID_FRAME 0x35F
#define SI_NAME_FRAME 0x35E

#define BMS_NAME "TinyBMS"

extern MCP_CAN *CAN;

uint8_t *make_charge_params_frame(uint16_t finalChargeVoltage, uint16_t maxChargeCurrent, uint16_t maxDischargeCurrent, uint16_t finalDischargeVoltage);
void parse_charge_params_frame(uint8_t frame[], uint16_t *finalChargeVoltage, uint16_t *maxChargeCurrent, uint16_t *maxDischargeCurrent, uint16_t *finalDischargeVoltage);
uint8_t *make_soc_frame(uint16_t stateOfCharge, uint16_t stateOfHealth, uint16_t stateOfChargeHighPrecision);
void parse_soc_frame(uint8_t frame[], uint16_t *stateOfCharge, uint16_t *stateOfHealth, uint16_t *stateOfChargeHighPrecision);
uint8_t *make_voltage_frame(uint16_t batteryVoltage, uint16_t batteryCurrent, uint16_t batteryTemp);
void parse_voltage_frame(uint8_t frame[], uint16_t *batteryVoltage, uint16_t *batteryCurrent, uint16_t *batteryTemp);
uint8_t *make_charge_params_frame2(uint16_t finalChargeVoltage, uint16_t maxChargeCurrent, uint16_t maxDischargeCurrent, uint16_t finalDischargeVoltage);
uint8_t *make_charge_params_frame3(uint16_t finalChargeVoltage, uint16_t maxChargeCurrent, uint16_t maxDischargeCurrent, uint16_t finalDischargeVoltage);

void init_sunnyisland();
void send_voltage_frame(Battery_voltage *voltage);
void send_name_frame();
void send_id_frame(uint16_t batteryCapacity);
void send_soc_frame(Battery_soc *soc);

#endif

