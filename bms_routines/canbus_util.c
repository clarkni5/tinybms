#include "canbus_util.h"

uint8_t *array_dup(uint8_t *src, uint16_t size) {
    
    uint8_t *result = calloc(sizeof(uint8_t), size);
    
    memcpy(result, src, size);
    
    return result;
    
}

uint8_t* make_charge_params_frame(uint16_t finalChargeVoltage,
		uint16_t maxChargeCurrent, uint16_t maxDischargeCurrent,
		uint16_t finalDischargeVoltage) {

	uint16_t frame[] = { leWord(finalChargeVoltage), leWord(maxChargeCurrent),
			leWord(maxDischargeCurrent), leWord(finalDischargeVoltage) };

	return array_dup((uint8_t*) frame, sizeof(frame));

}

void parse_charge_params_frame(uint8_t frame[], uint16_t *finalChargeVoltage, uint16_t *maxChargeCurrent, uint16_t *maxDischargeCurrent, uint16_t *finalDischargeVoltage) {

    *finalChargeVoltage = loHi(frame[0], frame[1]);
    *maxChargeCurrent = loHi(frame[2], frame[3]);
    *maxDischargeCurrent = loHi(frame[4], frame[5]);
    *finalDischargeVoltage = loHi(frame[6], frame[7]);

}

uint8_t* make_soc_frame(uint16_t stateOfCharge, uint16_t stateOfHealth,
		uint16_t stateOfChargeHighPrecision) {

	uint16_t frame[] = { leWord(stateOfCharge), leWord(stateOfHealth), leWord(
			stateOfChargeHighPrecision) };

	return array_dup((uint8_t*) frame, sizeof(frame));

}

void parse_soc_frame(uint8_t frame[], uint16_t *stateOfCharge, uint16_t *stateOfHealth, uint16_t *stateOfChargeHighPrecision) {

    *stateOfCharge = loHi(frame[0], frame[1]);
    *stateOfHealth = loHi(frame[2], frame[3]);
    *stateOfChargeHighPrecision = loHi(frame[4], frame[5]);

}

uint8_t* make_voltage_frame(uint16_t batteryVoltage, uint16_t batteryCurrent,
		uint16_t batteryTemp) {

	uint16_t frame[] = { leWord(batteryVoltage), leWord(batteryCurrent), leWord(
			batteryTemp) };

	return array_dup((uint8_t*) frame, sizeof(frame));

}

void parse_voltage_frame(uint8_t frame[], uint16_t *batteryVoltage, uint16_t *batteryCurrent, uint16_t *batteryTemp) {

    *batteryVoltage = loHi(frame[0], frame[1]);
    *batteryCurrent = loHi(frame[2], frame[3]);
    *batteryTemp = loHi(frame[4], frame[5]);

}
