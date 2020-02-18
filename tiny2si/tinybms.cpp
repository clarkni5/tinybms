#include "tinybms.h"
#include "util.h"

AltSoftSerial *softSerial;
ModbusMaster *modbus;

void init_tinybms() {

	softSerial = new AltSoftSerial(MODBUS_RX_PIN, MODBUS_TX_PIN);
	modbus = new ModbusMaster();

	// Init the modbus interface
	softSerial->begin(MODBUS_BAUD);
	modbus->begin(TINYBMS_DEVICE_ID, *softSerial);

}

int readRegistersWithRetry(uint8_t idx, uint8_t count, uint16_t *dest,
		uint8_t retrcnt) {

	do {

		if (modbus->readHoldingRegisters(idx, count) == modbus->ku8MBSuccess) {
			uint8_t j = 0;
			for (j = 0; j < count; j++) {
				dest[j] = modbus->getResponseBuffer(j);
				DEBUGP("register %hhu = %hx\r\n", j, dest[j]);
			}
			break;
		}

		retrcnt--;
		delay(MODBUS_RETRY_INTERVAL);

	} while (retrcnt > 0);

	return retrcnt - 1;
}

int load_battery_voltage(Battery_config *config, Battery_voltage *voltage) {

	int result = 1;

	if (readRegistersWithRetry(MAX_CELL_COUNT - config->cell_count,
			config->cell_count, voltage->cell_voltage,
			MODBUS_RETRY_COUNT) <= 0)
		result = -1;

	delay(MODBUS_INTERVAL);

	if (readRegistersWithRetry(PACK_VOLTAGE_REGISTER_0, 2,
			voltage->pack_voltage.ivoltage,
			MODBUS_RETRY_COUNT) <= 0)
		result = -1;

	delay(MODBUS_INTERVAL);

	if (readRegistersWithRetry(MIN_CELL_VOLTAGE_REGISTER, 1,
			&voltage->min_cell_voltage, MODBUS_RETRY_COUNT) >= 0)
		result = -1;

	delay(MODBUS_INTERVAL);

	if (readRegistersWithRetry(MAX_CELL_VOLTAGE_REGISTER, 1,
			&voltage->max_cell_voltage, MODBUS_RETRY_COUNT) >= 0)
		result = -1;

	if (result == 1)
		voltage->last_success = millis();

	delay(MODBUS_INTERVAL);

	return result;

}

int load_battery_current(Battery_current *current) {

	int result = 1;

	if (readRegistersWithRetry(PACK_CURRENT_REGISTER_0, 2,
			(uint16_t*) &current->pack_current,
			MODBUS_RETRY_COUNT) <= 0)
		result = -1;

	if (readRegistersWithRetry(MAX_DISCHARGE_CURRENT_REGISTER, 1,
			(uint16_t*) &current->max_discharge_current,
			MODBUS_RETRY_COUNT) <= 0)
		result = -1;

	if (readRegistersWithRetry(MAX_CHARGE_CURRENT_REGISTER, 1,
			(uint16_t*) &current->max_charge_current,
			MODBUS_RETRY_COUNT) <= 0)
		result = -1;

	if (result == 1)
		current->last_success = millis();

	delay(MODBUS_INTERVAL);

	return result;

}

int load_battery_config(Battery_config *config) {

	int result = 1;

	if (readRegistersWithRetry(CELL_COUNT_REGISTER, 1, &config->cell_count, 10)
			<= 0) {
		DEBUGP("Couldn't load cell count\r\n");
		result = -1;
	}

	delay(MODBUS_INTERVAL);

	if (readRegistersWithRetry(PACK_CAPACITY_REGISTER, 1, &config->capacity,
	MODBUS_RETRY_COUNT) <= 0) {
		DEBUGP("Couldn't load pack capacity\r\n");
		result = -1;
	}

	if (result == 1)
		config->last_success = millis();

	delay(MODBUS_INTERVAL);

	return result;

}

int load_battery_soc(Battery_soc *soc) {

	int result = 1;
	uint32_t stateOfCharge;

	if (readRegistersWithRetry(PACK_SOC_REGISTER_0, 2,
			(uint16_t*) &stateOfCharge,
			MODBUS_RETRY_COUNT) <= 0)
		result = -1;
	else
		soc->stateOfCharge = (uint16_t) (stateOfCharge / 1000000);

	if (result == 1)
		soc->last_success = millis();

	delay(MODBUS_INTERVAL);

	return result;

}
