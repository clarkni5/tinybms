#include "sunnyisland.h"

MCP_CAN *CAN;

void init_sunnyisland() {

	CAN = new MCP_CAN(CAN_CS_PIN);

	while (CAN->begin(CAN_500KBPS, MCP_8MHz) != CAN_OK) {
		serial_bprintf(buf, "Unable to start CAN bus\r\n");
		delay(1000);
	}

}

uint8_t* make_safety_frame(uint16_t finalChargeVoltage,
		uint16_t maxChargeCurrent, uint16_t maxDischargeCurrent,
		uint16_t finalDischargeVoltage) {

	uint16_t frame[] = {
		leWord(finalChargeVoltage),
		leWord(maxChargeCurrent),
		leWord(maxDischargeCurrent),
		leWord(finalDischargeVoltage)
	};

	return array_dup((uint8_t*) frame, sizeof(frame));

}

/*
 void parse_charge_params_frame(uint8_t frame[], uint16_t *finalChargeVoltage, uint16_t *maxChargeCurrent, uint16_t *maxDischargeCurrent, uint16_t *finalDischargeVoltage) {

 *finalChargeVoltage = loHi(frame[0], frame[1]);
 *maxChargeCurrent = loHi(frame[2], frame[3]);
 *maxDischargeCurrent = loHi(frame[4], frame[5]);
 *finalDischargeVoltage = loHi(frame[6], frame[7]);

 }
 */

uint8_t* make_soc_frame(uint16_t stateOfCharge, uint16_t stateOfHealth,
		uint16_t stateOfChargeHighPrecision) {

	uint16_t frame[] = {
		leWord(stateOfCharge),
		leWord(stateOfHealth),
		leWord(stateOfChargeHighPrecision)
	};

	return array_dup((uint8_t*) frame, sizeof(frame));

}

/*
 void parse_soc_frame(uint8_t frame[], uint16_t *stateOfCharge, uint16_t *stateOfHealth, uint16_t *stateOfChargeHighPrecision) {

 *stateOfCharge = loHi(frame[0], frame[1]);
 *stateOfHealth = loHi(frame[2], frame[3]);
 *stateOfChargeHighPrecision = loHi(frame[4], frame[5]);

 }
 */

uint8_t* make_sensor_frame(uint16_t batteryVoltage, uint16_t batteryCurrent, uint16_t batteryTemp) {

	uint16_t frame[] = {
		leWord(batteryVoltage),
		leWord(batteryCurrent),
		leWord(batteryTemp)
	};

	return array_dup((uint8_t*) frame, sizeof(frame));

}

/*
 void parse_voltage_frame(uint8_t frame[], uint16_t *batteryVoltage, uint16_t *batteryCurrent, uint16_t *batteryTemp) {

 *batteryVoltage = loHi(frame[0], frame[1]);
 *batteryCurrent = loHi(frame[2], frame[3]);
 *batteryTemp = loHi(frame[4], frame[5]);

 }
 */

void send_si_safety_frame(Battery_config *battery_config, Battery_safety_params *battery_safety,
		Battery_current *battery_current,
		void (*onsend_callback)(uint32_t id, uint8_t frame[], uint8_t len)) {

	uint8_t *frame = make_safety_frame(
			// TinyBMS Manual 4.3. Battery Insider cell settings
			// Under-Voltage Cutoff < Fully Discharged Voltage < Fully Charged Voltage < Over-Voltage Cutoff
			// ---
			// TinyBMS LSB = 0.001 V, Sunny Island LSB = 0.1 V
			(battery_safety->cell_charged_v / 1000.0) * battery_config->cell_count * 10,

			// TinyBMS Manual 3.1.7. Charging Over-Current Fault
			// Normal condition: Charging Current <= Charge Over-Current Cutoff threshold.
			// Fault condition: Charging Current > Charge Over-Current Cutoff threshold.
			// ---
			// TinyBMS LSB = 1 A, Sunny Island LSB = 0.1 A
			battery_current->charge_overcurrent_cutoff * 10,

			// TinyBMS Manual 3.1.8. Discharging Over-Current Fault
			// Normal condition: Discharging Current <= Discharge Over-Current Cutoff threshold.
			// Fault condition: Discharging Current > Discharge Over-Current Cutoff threshold.
			// ---
			// TinyBMS LSB = 1 A, Sunny Island LSB = 0.1 A
			battery_current->discharge_overcurrent_cutoff * 10,

			// TinyBMS Manual 4.3. Battery Insider cell settings
			// Under-Voltage Cutoff < Fully Discharged Voltage < Fully Charged Voltage < Over-Voltage Cutoff
			// ---
			// TinyBMS LSB = 0.001 V, Sunny Island LSB = 0.1 V
			(battery_safety->cell_discharged_v  / 1000.0) * battery_config->cell_count * 10
		);
	CAN->sendMsgBuf(SI_SAFETY_FRAME, 0, 8, frame);
	(*onsend_callback)(SI_SAFETY_FRAME, frame, 8);
	free(frame);
	delay(CAN_DELAY);

}

void send_si_soc_frame(Battery_soc *soc,
		void (*onsend_callback)(uint32_t id, uint8_t frame[], uint8_t len)) {

	uint8_t *frame = make_soc_frame(
			// TinyBMS LSB = 1 %, Sunny Island LSB = 1 %
			soc->stateOfCharge,
			// TinyBMS LSB = 1 %, Sunny Island LSB = 1 %
			soc->stateOfHealth,
			// TinyBMS LSB = 0.000001 %, Sunny Island LSB = 0.01 %
			soc->stateOfChargeHp / 10000
		);
	CAN->sendMsgBuf(SI_SOC_FRAME, 0, 6, frame);
	(*onsend_callback)(SI_SOC_FRAME, frame, 6);
	free(frame);
	delay(CAN_DELAY);

}

int16_t get_temperature(Battery_temp *battery_temp) {

	int16_t temperature = INT16_MIN;

	if (battery_temp->external_temp_1 > INT16_MIN && battery_temp->external_temp_1 > INT16_MIN) {
		// Both external temperature sensor are connected; use the minimum value
		temperature = min(battery_temp->external_temp_1, battery_temp->external_temp_2);

	} else if (battery_temp->external_temp_1 > INT16_MIN) {
		// External temperature sensor 1 is connected
		temperature = battery_temp->external_temp_1;

	} else if (battery_temp->external_temp_2 > INT16_MIN) {
		// External temperature sensor 2 is connected
		temperature = battery_temp->external_temp_2;

	} else {
		// No external temperature sensors are connected; use the onboard temperature sensor
		temperature = battery_temp->onboard_temp;
	}

	return temperature;

}

void send_si_sensor_frame(Battery_voltage *voltage, Battery_current *battery_current, Battery_temp *battery_temp,
		void (*onsend_callback)(uint32_t id, uint8_t frame[], uint8_t len)) {

	uint8_t *frame = make_sensor_frame(
			// TinyBMS LSB = 0.01 V, Sunny Island LSB = 0.01 V
			(uint16_t) (voltage->pack_voltage.fvoltage * 100),
			// TinyBMS LSB = 0.01 A, Sunny Island LSB = 0.1 A
			// Discharge current is positive
			(uint16_t) (battery_current->pack_current.fcurrent * 10 * -1),
			// TinyBMS LSB = 0.1 C, Sunny Island LSB = 0.1 C
			get_temperature(battery_temp)
		);
	CAN->sendMsgBuf(SI_SENSOR_FRAME, 0, 6, frame);
	(*onsend_callback)(SI_SENSOR_FRAME, frame, 6);
	free(frame);
	delay(CAN_DELAY);

}

void send_si_id_frame(void (*onsend_callback)(uint32_t id, uint8_t frame[], uint8_t len)) {

	CAN->sendMsgBuf(SI_ID_FRAME, 0, strlen(BMS_NAME), (unsigned char*) BMS_NAME);
	// Using sizeof - 1 instead of strlen to allow null (\0) characters to pad the name field
	(*onsend_callback)(SI_ID_FRAME, (unsigned char*) BMS_NAME, sizeof(BMS_NAME) - 1);
	delay(CAN_DELAY);

}

void send_si_system_frame(Bms_version *bms_version, uint16_t batteryCapacity,
		void (*onsend_callback)(uint32_t id, uint8_t frame[], uint8_t len)) {

	uint8_t frame[] = {
			0x4c,  // 0x4C = ASCII "L"
			0x69,  // 0x69 = ASCII "i"
			bms_version->hw_ver.hw_ch_version,
			bms_version->hw_ver.hw_version,
			// TinyBMS LSB = 1 Ah, Sunny Island LSB = 1 Ah
			lowByte(batteryCapacity),
			highByte(batteryCapacity),
			lowByte(bms_version->int_fw_ver),
			highByte(bms_version->int_fw_ver)
		};
	CAN->sendMsgBuf(SI_ABOUT_FRAME, 0, 8, frame);
	(*onsend_callback)(SI_ABOUT_FRAME, frame, 8);
	delay(CAN_DELAY);

}

void send_fault_frame(uint8_t f0, uint8_t f1, uint8_t f2, uint8_t f3,
		uint8_t w0, uint8_t w1, uint8_t w2, uint8_t w3,
		void (*onsend_callback)(uint32_t id, uint8_t frame[], uint8_t len)) {

	// TODO put in zero bytes for warnings
	uint8_t frame[] = { f0, f1, f2, f3, w0, w1, w2, w3 };
	CAN->sendMsgBuf(SI_FAULT_FRAME, 0, 8, frame);
	(*onsend_callback)(SI_FAULT_FRAME, frame, 8);
	delay(CAN_DELAY);

}

void send_si_faults(Battery_config *battery_config, Battery_voltage *battery_voltage, Battery_current *battery_current,
		Battery_safety_params *battery_safety,
		void (*onsend_callback)(uint32_t id, uint8_t frame[], uint8_t len)) {

	uint8_t f0 = 0;  // byte 0
	uint8_t f1 = 0;  // byte 1
	uint8_t f2 = 0;  // byte 2
	uint8_t f3 = 0;  // byte 3

	if (battery_config->cell_count > 0) {

		if (battery_voltage->min_cell_voltage < battery_safety->cell_discharged_v) {
			serial_bprintf(buf, "FAULT: undervoltage\r\n");
			f0 = f0 | FAULT0_UNDERVOLTAGE;
		}

		if (battery_voltage->max_cell_voltage > battery_safety->cell_charged_v) {
			serial_bprintf(buf, "FAULT: overvoltage\r\n");
			f0 = f0 | FAULT0_OVERVOLTAGE;
		}

		if (battery_current->pack_current.fcurrent * -1 > battery_current->discharge_overcurrent_cutoff) {
			serial_bprintf(buf, "FAULT: discharge overcurrent\r\n");
			f1 = f1 | FAULT1_DISCHARGE_OVERCURRENT;
		}

		if (battery_current->pack_current.fcurrent > battery_current->charge_overcurrent_cutoff) {
			serial_bprintf(buf, "FAULT: charge overcurrent\r\n");
			f2 = f2 | FAULT2_CHARGE_OVERCURRENT;
		}

	} else {
		serial_bprintf(buf, "FAULT: No battery cells found.\r\n");
		f2 = f2 | FAULT2_BMS_ERROR;
	}

	send_fault_frame(f0, f1, f2, f3, 0, 0, 0, 0, onsend_callback);

}
