#include "sunnyisland.h"

MCP_CAN *CAN;

uint8_t *array_dup(uint8_t *src, uint16_t size) {

    uint8_t *result = calloc(sizeof(uint8_t), size);
    memcpy(result, src, size);
    return result;

}

uint8_t *make_charge_params_frame(uint16_t finalChargeVoltage, uint16_t maxChargeCurrent, uint16_t maxDischargeCurrent, uint16_t finalDischargeVoltage) {

    uint16_t frame[] = { leWord(finalChargeVoltage), leWord(maxChargeCurrent), leWord(maxDischargeCurrent), leWord(finalDischargeVoltage)};

    return array_dup((uint8_t*)frame, sizeof(frame));

}

/*
void parse_charge_params_frame(uint8_t frame[], uint16_t *finalChargeVoltage, uint16_t *maxChargeCurrent, uint16_t *maxDischargeCurrent, uint16_t *finalDischargeVoltage) {

    *finalChargeVoltage = loHi(frame[0], frame[1]);
    *maxChargeCurrent = loHi(frame[2], frame[3]);
    *maxDischargeCurrent = loHi(frame[4], frame[5]);
    *finalDischargeVoltage = loHi(frame[6], frame[7]);

}
*/

uint8_t *make_soc_frame(uint16_t stateOfCharge, uint16_t stateOfHealth, uint16_t stateOfChargeHighPrecision) {

	uint16_t frame[] = { leWord(stateOfCharge), leWord(stateOfHealth), leWord(stateOfChargeHighPrecision) };

    return array_dup((uint8_t*)frame, sizeof(frame));

}

/*
void parse_soc_frame(uint8_t frame[], uint16_t *stateOfCharge, uint16_t *stateOfHealth, uint16_t *stateOfChargeHighPrecision) {

    *stateOfCharge = loHi(frame[0], frame[1]);
    *stateOfHealth = loHi(frame[2], frame[3]);
    *stateOfChargeHighPrecision = loHi(frame[4], frame[5]);

}
*/

uint8_t *make_voltage_frame(uint16_t batteryVoltage, uint16_t batteryCurrent, uint16_t batteryTemp) {

	uint16_t frame[] = { leWord(batteryVoltage), leWord(batteryCurrent), leWord(batteryTemp) };

    return array_dup((uint8_t*)frame, sizeof(frame));

}

/*
void parse_voltage_frame(uint8_t frame[], uint16_t *batteryVoltage, uint16_t *batteryCurrent, uint16_t *batteryTemp) {

    *batteryVoltage = loHi(frame[0], frame[1]);
    *batteryCurrent = loHi(frame[2], frame[3]);
    *batteryTemp = loHi(frame[4], frame[5]);

}
*/

void init_sunnyisland() {

	CAN = new MCP_CAN(CAN_PIN);

	while(CAN->begin(CAN_500KBPS, MCP_8MHz) != CAN_OK) {
		serial_bprintf(buf, "Unable to start CAN bus\n");
		delay(1000);
	}

}

void send_charge_params_frame() {

	uint8_t *frame = make_charge_params_frame(0, 0, 0, 0);
	CAN->sendMsgBuf(SI_CHARGE_PARAMS_FRAME, 0, 8, frame);
	free(frame);
	delay(CAN_DELAY);

}

void send_voltage_frame(Battery_voltage *voltage) {

	uint8_t *frame = make_voltage_frame((uint16_t)voltage->pack_voltage.fvoltage, 0, 0);
	CAN->sendMsgBuf(SI_VOLTAGE_FRAME, 0, 8, frame);
	free(frame);
	delay(CAN_DELAY);

}

void send_name_frame() {

	CAN->sendMsgBuf(SI_NAME_FRAME, 0, strlen(BMS_NAME), (unsigned char*)BMS_NAME);
	delay(CAN_DELAY);

}

void send_id_frame(uint16_t batteryCapacity) {

	unsigned char frame[] = { 0x4c, 0x69, 0x04, 0x0f, lowByte(batteryCapacity), highByte(batteryCapacity), 0x11, 0x17 };
	CAN->sendMsgBuf(SI_ID_FRAME, 0, 8, frame);
	delay(CAN_DELAY);

}

void send_soc_frame(Battery_soc *soc) {

	uint8_t *frame = make_soc_frame(soc->stateOfCharge, soc->stateOfHealth, soc->stateOfChargeHp);
	CAN->sendMsgBuf(SI_SOC_FRAME, 0, 6, frame);
	free(frame);
	delay(CAN_DELAY);

}
