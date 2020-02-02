#include "canbus_util.h"

unsigned char *make_charge_params_frame(unsigned long finalChargeVoltage, long maxChargeCurrent, long maxDischargeCurrent, long finalDischargeVoltage) {

    unsigned char *result = malloc(6);
    unsigned char *p = result;
    
    *p++ = lowByte(finalChargeVoltage);
    *p++ = highByte(finalChargeVoltage);
    *p++ = lowByte(maxChargeCurrent);
    *p++ = highByte(maxChargeCurrent);
    *p++ = lowByte(finalDischargeVoltage);
    *p++ = highByte(finalDischargeVoltage);

    return result;

}

void parse_charge_params_frame(unsigned char frame[], unsigned long *finalChargeVoltage, long *maxChargeCurrent, long *maxDischargeCurrent, long *finalDischargeVoltage) {
    
    *finalChargeVoltage = loHi(frame[0], frame[1]);
    *maxChargeCurrent = loHi(frame[2], frame[3]);
    *maxDischargeCurrent = loHi(frame[4], frame[5]);
    *finalDischargeVoltage = loHi(frame[6], frame[7]);
    
}
