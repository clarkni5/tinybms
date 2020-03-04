#include <Arduino.h>
#include "util.h"
#include "tinybms.h"
#include "sunnyisland.h"

#define SERIAL_BAUD 9600
#define POLL_INTERVAL 10000

#define print_age(x,y) 	(serial_bprintf(buf, "%s data is %us old\r\n", x, (millis() - y) / 1000))

Battery_config battery_config;
Battery_voltage battery_voltage;
Battery_current battery_current;
Battery_soc battery_soc;
Battery_safety_params battery_safety;
Bms_version bms_version;

char buf[128];

void setup() {

	Serial.begin(SERIAL_BAUD);
	serial_bprintf(buf, "Starting tiny2si\r\n");

	init_tinybms();
//	reset_tinybms();
	init_sunnyisland();

	delay(2000);
	serial_bprintf(buf, "Init OK\r\n");

}

void load_battery_data() {

	if (bms_version.last_success == 0) {
		load_bms_version(&bms_version);
	}

	if (battery_config.last_success == 0) {
		load_battery_config(&battery_config);
	}
	load_battery_voltage(&battery_config, &battery_voltage);
	load_battery_current(&battery_current);
	load_battery_soc(&battery_soc);
	load_battery_safety(&battery_safety);

}

void dump_battery_data() {

	uint8_t k;
	char cls[] = { 27, '[', '2', 'J', 27, '[', 'H', 0 };

	serial_bprintf(buf, cls);

	print_age("config", battery_config.last_success);

	serial_bprintf(buf, "%u configured cells\r\n", battery_config.cell_count);
	serial_bprintf(buf, "capacity %uAh\r\n", battery_config.capacity);

	for (k = 0; k < battery_config.cell_count; k++) {
		serial_bprintf(buf, "Cell %hhu voltage: %3.2fV\r\n",
				battery_config.cell_count - k,
				battery_voltage.cell_voltage[k] / 10000.0);
	}

	print_age("voltage", battery_voltage.last_success);

	serial_bprintf(buf, "Pack voltage: %3.2fV\r\n",
			battery_voltage.pack_voltage.fvoltage);

	serial_bprintf(buf, "Min cell voltage: %3.2fV\r\n",
			battery_voltage.min_cell_voltage / 1000.0);
	serial_bprintf(buf, "Max cell voltage: %3.2fV\r\n",
			battery_voltage.max_cell_voltage / 1000.0);

	serial_bprintf(buf, "\r\n");

	print_age("current", battery_current.last_success);

	serial_bprintf(buf, "Pack current: %3.2fA\r\n",
			battery_current.pack_current);
	serial_bprintf(buf, "Max discharge current: %uA\r\n",
			battery_current.max_discharge_current);
	serial_bprintf(buf, "Max charge current: %uA\r\n",
			battery_current.max_charge_current);

	serial_bprintf(buf, "\r\n");

	print_age("soc", battery_current.last_success);

	serial_bprintf(buf, "Pack SOC: %hu%%\r\n", battery_soc.stateOfCharge);
	serial_bprintf(buf, "Pack SOC hp: %lu\r\n", battery_soc.stateOfChargeHp);
	serial_bprintf(buf, "Pack SOH : %hu%%\r\n", battery_soc.stateOfHealth);

	serial_bprintf(buf, "\r\n");

	print_age("safety", battery_safety.last_success);

	serial_bprintf(buf, "Cell fully charged: %3.2fV\r\n",
			battery_safety.cell_charged_v / 1000.0);
	serial_bprintf(buf, "Cell fully discharged: %3.2fV\r\n",
			battery_safety.cell_discharged_v / 1000.0);

	serial_bprintf(buf, "\r\n");

	print_age("BMS ver", bms_version.last_success);

	serial_bprintf(buf,
			"Hardware Version %hhu, Hardware Changes Version %hu\r\n",
			bms_version.hw_ver.hw_version, bms_version.hw_ver.hw_ch_version);
	serial_bprintf(buf,
			"Public Release Firmware Version %hhu, BPT %hhhx BCS %hhhx\r\n",
			bms_version.fw_ver.fw_version, bms_version.fw_ver.bpt,
			bms_version.fw_ver.bcs);
	serial_bprintf(buf, "Internal Firmware Version %u\r\n",
			bms_version.int_fw_ver);
	serial_bprintf(buf, "Bootloader version %hu, Profile Version %hu\r\n",
			bms_version.loader_ver.booloader_ver,
			bms_version.loader_ver.profile_ver);

	uint8_t *s = bms_version.serial_num;
	serial_bprintf(buf, "Product Serial Number %hhx-%hhx-%hhx-%hhx-%hhx-%hhx-%hhx-%hhx-%hhx-%hhx-%hhx-%hhx\r\n", s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7], s[8], s[9], s[10], s[11]);

	serial_bprintf(buf, "\r\n");

}

void send_battery_data() {

	send_name_frame();
	send_id_frame(battery_config.capacity);
	send_soc_frame(&battery_soc);
	send_voltage_frame(&battery_voltage);

}

void send_battery_faults() {

	if (battery_voltage.min_cell_voltage < battery_safety.cell_discharged_v) {
		serial_bprintf(buf, "FAULT: undervoltage\r\n");
		send_fault_frame(FAULT0_UNDERVOLTAGE, 0, 0, 0);
	}

	if (battery_voltage.max_cell_voltage > battery_safety.cell_charged_v) {
		serial_bprintf(buf, "FAULT: overvoltage\r\n");
		send_fault_frame(FAULT0_OVERVOLTAGE, 0, 0, 0);
	}

	if (battery_current.pack_current * -1
			> battery_current.max_discharge_current) {
		serial_bprintf(buf, "FAULT: discharge overcurrent\r\n");
		send_fault_frame(0, FAULT1_DISCHARGE_OVERCURRENT, 0, 0);
	}

	if (battery_current.pack_current > battery_current.max_charge_current) {
		serial_bprintf(buf, "FAULT: charge overcurrent\r\n");
		send_fault_frame(0, 0, FAULT2_CHARGE_OVERCURRENT, 0);
	}

}

void loop() {

	load_battery_data();

	if (battery_config.cell_count > 0) {

		dump_battery_data();
		send_battery_data();
		send_battery_faults();

	}

	serial_bprintf(buf, "Uptime: %us\r\n", millis() / 1000);
	serial_bprintf(buf, "Free memory: %u\r\n", freeMemory());

	delay(POLL_INTERVAL);

}
