#include "canbus_util.h"

byte *make_charge_params_frame(word finalChargeVoltage, word maxChargeCurrent, word maxDischargeCurrent, word finalDischargeVoltage) {

    byte *result = calloc(sizeof (byte), 6);
    byte *p = result;

    *p++ = lowByte(finalChargeVoltage);
    *p++ = highByte(finalChargeVoltage);
    *p++ = lowByte(maxChargeCurrent);
    *p++ = highByte(maxChargeCurrent);
    *p++ = lowByte(finalDischargeVoltage);
    *p++ = highByte(finalDischargeVoltage);

    return result;

}

byte *make_charge_params_frame2(word finalChargeVoltage, word maxChargeCurrent, word maxDischargeCurrent, word finalDischargeVoltage) {

    word *result = calloc(sizeof (word), 3);
    word *p = result;

    *p++ = leWord(finalChargeVoltage);
    *p++ = leWord(maxChargeCurrent);
    *p++ = leWord(finalDischargeVoltage);

    return (byte*)result;

}

void parse_charge_params_frame(byte frame[], word *finalChargeVoltage, word *maxChargeCurrent, word *maxDischargeCurrent, word *finalDischargeVoltage) {

    *finalChargeVoltage = loHi(frame[0], frame[1]);
    *maxChargeCurrent = loHi(frame[2], frame[3]);
    *maxDischargeCurrent = loHi(frame[4], frame[5]);
    *finalDischargeVoltage = loHi(frame[6], frame[7]);

}

byte *make_soc_frame(word stateOfCharge, word stateOfHealth, word stateOfChargeHighPrecision) {

    byte *result = calloc(sizeof (byte), 6);
    byte *p = result;

    *p++ = lowByte(stateOfCharge);
    *p++ = highByte(stateOfCharge);
    *p++ = lowByte(stateOfHealth);
    *p++ = highByte(stateOfHealth);
    *p++ = lowByte(stateOfChargeHighPrecision);
    *p++ = highByte(stateOfChargeHighPrecision);

    return result;

}

void parse_soc_frame(byte frame[], word *stateOfCharge, word *stateOfHealth, word *stateOfChargeHighPrecision) {

    *stateOfCharge = loHi(frame[0], frame[1]);
    *stateOfHealth = loHi(frame[2], frame[3]);
    *stateOfChargeHighPrecision = loHi(frame[4], frame[5]);

}

byte *make_voltage_frame(word batteryVoltage, word batteryCurrent, word batteryTemp) {

    byte *result = calloc(sizeof (byte), 6);
    byte *p = result;

    *p++ = lowByte(batteryVoltage);
    *p++ = highByte(batteryVoltage);
    *p++ = lowByte(batteryCurrent);
    *p++ = highByte(batteryCurrent);
    *p++ = lowByte(batteryTemp);
    *p++ = highByte(batteryTemp);

    return result;

}

void parse_voltage_frame(byte frame[], word *batteryVoltage, word *batteryCurrent, word *batteryTemp) {

    *batteryVoltage = loHi(frame[0], frame[1]);
    *batteryCurrent = loHi(frame[2], frame[3]);
    *batteryTemp = loHi(frame[4], frame[5]);

}
