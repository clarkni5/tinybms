#include "sunnyisland.h"

MCP_CAN *CAN;

uint8_t *array_dup(uint8_t *src, uint16_t size) {

    uint8_t *result = calloc(sizeof(uint8_t), size);
    memcpy(result, src, size);
    return result;

}

uint8_t *make_charge_params_frame(uint16_t finalChargeVoltage, uint16_t maxChargeCurrent, uint16_t maxDischargeCurrent, uint16_t finalDischargeVoltage) {

    uint8_t *result = calloc(sizeof (uint8_t), 6);
    uint8_t *p = result;

    *p++ = lowByte(finalChargeVoltage);
    *p++ = highByte(finalChargeVoltage);
    *p++ = lowByte(maxChargeCurrent);
    *p++ = highByte(maxChargeCurrent);
    *p++ = lowByte(finalDischargeVoltage);
    *p++ = highByte(finalDischargeVoltage);

    return result;

}

uint8_t *make_charge_params_frame2(uint16_t finalChargeVoltage, uint16_t maxChargeCurrent, uint16_t maxDischargeCurrent, uint16_t finalDischargeVoltage) {

    uint16_t *result = calloc(sizeof (uint16_t), 3);
    uint16_t *p = result;

    *p++ = leWord(finalChargeVoltage);
    *p++ = leWord(maxChargeCurrent);
    *p++ = leWord(finalDischargeVoltage);

    return (uint8_t*)result;

}

uint8_t *make_charge_params_frame3(uint16_t finalChargeVoltage, uint16_t maxChargeCurrent, uint16_t maxDischargeCurrent, uint16_t finalDischargeVoltage) {

    uint16_t frame[] = {leWord(finalChargeVoltage), leWord(maxChargeCurrent), leWord(maxDischargeCurrent), leWord(finalDischargeVoltage)};

    return array_dup((uint8_t*)frame, sizeof(frame));

}

void parse_charge_params_frame(uint8_t frame[], uint16_t *finalChargeVoltage, uint16_t *maxChargeCurrent, uint16_t *maxDischargeCurrent, uint16_t *finalDischargeVoltage) {

    *finalChargeVoltage = loHi(frame[0], frame[1]);
    *maxChargeCurrent = loHi(frame[2], frame[3]);
    *maxDischargeCurrent = loHi(frame[4], frame[5]);
    *finalDischargeVoltage = loHi(frame[6], frame[7]);

}

uint8_t *make_soc_frame(uint16_t stateOfCharge, uint16_t stateOfHealth, uint16_t stateOfChargeHighPrecision) {

    uint8_t *result = calloc(sizeof (uint8_t), 6);
    uint8_t *p = result;

    *p++ = lowByte(stateOfCharge);
    *p++ = highByte(stateOfCharge);
    *p++ = lowByte(stateOfHealth);
    *p++ = highByte(stateOfHealth);
    *p++ = lowByte(stateOfChargeHighPrecision);
    *p++ = highByte(stateOfChargeHighPrecision);

    return result;

}

void parse_soc_frame(uint8_t frame[], uint16_t *stateOfCharge, uint16_t *stateOfHealth, uint16_t *stateOfChargeHighPrecision) {

    *stateOfCharge = loHi(frame[0], frame[1]);
    *stateOfHealth = loHi(frame[2], frame[3]);
    *stateOfChargeHighPrecision = loHi(frame[4], frame[5]);

}

uint8_t *make_voltage_frame(uint16_t batteryVoltage, uint16_t batteryCurrent, uint16_t batteryTemp) {

    uint8_t *result = calloc(sizeof (uint8_t), 6);
    uint8_t *p = result;

    *p++ = lowByte(batteryVoltage);
    *p++ = highByte(batteryVoltage);
    *p++ = lowByte(batteryCurrent);
    *p++ = highByte(batteryCurrent);
    *p++ = lowByte(batteryTemp);
    *p++ = highByte(batteryTemp);

    return result;

}

void parse_voltage_frame(uint8_t frame[], uint16_t *batteryVoltage, uint16_t *batteryCurrent, uint16_t *batteryTemp) {

    *batteryVoltage = loHi(frame[0], frame[1]);
    *batteryCurrent = loHi(frame[2], frame[3]);
    *batteryTemp = loHi(frame[4], frame[5]);

}

void init_sunnyisland() {

	CAN = new MCP_CAN(CAN_PIN);

	while(CAN->begin(CAN_500KBPS, MCP_8MHz) != CAN_OK) {
		serial_bprintf(buf, "Unable to start CAN bus\n");
		delay(100);
	}

}

void send_charge_params_frame() {

	uint8_t *frame = make_charge_params_frame(0, 0, 0, 0);
	CAN->sendExtMsgBuf(SI_CHARGE_PARAMS_FRAME, 0, 8, frame);
	free(frame);
	delay(210);

}

void send_voltage_frame(Battery_voltage *voltage) {

	uint8_t *frame = make_voltage_frame((uint16_t)voltage->pack_voltage.fvoltage, 0, 0);
	CAN->sendExtMsgBuf(SI_VOLTAGE_FRAME, 0, 8, frame);
	free(frame);
	delay(210);

}
