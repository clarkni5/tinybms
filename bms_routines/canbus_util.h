#ifndef CANBUS_UTIL_H
#define CANBUS_UTIL_H

#include "arduino_util.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned char *make_charge_params_frame(unsigned long finalChargeVoltage, long maxChargeCurrent, long maxDischargeCurrent, long finalDischargeVoltage);
void parse_charge_params_frame(unsigned char frame[], unsigned long *finalChargeVoltage, long *maxChargeCurrent, long *maxDischargeCurrent, long *finalDischargeVoltage);

#ifdef __cplusplus
}
#endif

#endif /* CANBUS_UTIL_H */

