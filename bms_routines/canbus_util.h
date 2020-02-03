#ifndef CANBUS_UTIL_H
#define CANBUS_UTIL_H

#include "arduino_util.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned char *make_charge_params_frame(word finalChargeVoltage, word maxChargeCurrent, word maxDischargeCurrent, word finalDischargeVoltage);
void parse_charge_params_frame(byte frame[], word *finalChargeVoltage, word *maxChargeCurrent, word *maxDischargeCurrent, word *finalDischargeVoltage);

#ifdef __cplusplus
}
#endif

#endif /* CANBUS_UTIL_H */

