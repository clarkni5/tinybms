#ifndef CANBUS_UTIL_H
#define CANBUS_UTIL_H

#include "arduino_util.h"

#ifdef __cplusplus
extern "C" {
#endif

byte *make_charge_params_frame(word finalChargeVoltage, word maxChargeCurrent, word maxDischargeCurrent, word finalDischargeVoltage);
void parse_charge_params_frame(byte frame[], word *finalChargeVoltage, word *maxChargeCurrent, word *maxDischargeCurrent, word *finalDischargeVoltage);
byte *make_soc_frame(word stateOfCharge, word stateOfHealth, word stateOfChargeHighPrecision);
void parse_soc_frame(byte frame[], word *stateOfCharge, word *stateOfHealth, word *stateOfChargeHighPrecision);
byte *make_voltage_frame(word batteryVoltage, word batteryCurrent, word batteryTemp);
void parse_voltage_frame(byte frame[], word *batteryVoltage, word *batteryCurrent, word *batteryTemp);

#ifdef __cplusplus
}
#endif

#endif /* CANBUS_UTIL_H */

