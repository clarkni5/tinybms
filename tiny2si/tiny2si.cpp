#include <Arduino.h>
#include "sunnyisland.h"
#include "telemetry.h"
#include "tinybms.h"
#include "util.h"

#define SERIAL_BAUD 9600

#define LOOP_PROCESS_INTERVAL 10000  // milliseconds

// The Tiny BMS measures battery pack current at a rate of 10 Hz (every 100 ms)
#define LOOP_SAMPLING_INTERVAL 100  // milliseconds

#define print_age(x,y) 	(serial_bprintf(buf, "%s data is %us old\r\n", x, (millis() - y) / 1000))

Battery_config battery_config;
Battery_voltage battery_voltage;
Battery_current battery_current;
Battery_temp battery_temp;
Battery_soc battery_soc;
Battery_safety_params battery_safety;
Bms_version bms_version;
Battery_sampling battery_sampling;

char buf[128];

uint16_t loop_duration = 0;
int free_memory = 0;

unsigned long next_process_time = 0;
unsigned long next_sampling_time = 0;

void setup() {

	Serial.begin(SERIAL_BAUD);
	serial_bprintf(buf, "Starting tiny2si\r\n");

	init_tinybms();
//	reset_tinybms();
	init_sunnyisland();
	init_telemetry();

	delay(2000);

	free_memory = get_free_memory();

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
	load_battery_temp(&battery_temp);
	load_battery_soc(&battery_soc);
	load_battery_safety(&battery_safety);

	// The battery pack current can fluctuate a lot over a short period of time, so we use the sampling data to
	// calculate the average current.
	float avg_current = 0;
	if (battery_sampling.pack_current_count > 0) {
		avg_current = battery_sampling.pack_current_sum / battery_sampling.pack_current_count;
	}
	battery_current.pack_current.fcurrent = avg_current;

}

void reset_sampling_data() {

	battery_sampling.pack_current_sum = 0;
	battery_sampling.pack_current_count = 0;

}

void load_battery_sampling_data() {

	sample_battery_pack_current(&battery_sampling);

}

void print_data() {

	uint8_t k;

	// ANSI escape sequence to clear the screen and move the cursor to the
	// upper left corner.
	char cls[] = { 27, '[', '2', 'J', 27, '[', 'H', 0 };
	serial_bprintf(buf, cls);

	print_age("Config", battery_config.last_success);

	serial_bprintf(buf, "%hu configured cells\r\n", battery_config.cell_count);
	serial_bprintf(buf, "Capacity %huAh\r\n", battery_config.capacity);

	for (k = 0; k < battery_config.cell_count; k++) {
		serial_bprintf(buf, "Cell %hu voltage: %4.2fV\r\n",
				battery_config.cell_count - k,
				battery_voltage.cell_voltage[k] / 10000.0);
	}

	print_age("Voltage", battery_voltage.last_success);

	serial_bprintf(buf, "Pack voltage: %4.2fV\r\n", battery_voltage.pack_voltage.fvoltage);
	serial_bprintf(buf, "Min cell voltage: %4.2fV, max cell voltage: %4.2fV\r\n",
			battery_voltage.min_cell_voltage / 1000.0,
			battery_voltage.max_cell_voltage / 1000.0);

	serial_bprintf(buf, "\r\n");

	print_age("Current", battery_current.last_success);

	serial_bprintf(buf, "Pack current: %4.2fA\r\n", battery_current.pack_current.fcurrent);
	serial_bprintf(buf, "Max charge current: %huA, max discharge current: %huA\r\n",
			battery_current.charge_overcurrent_cutoff,
			battery_current.discharge_overcurrent_cutoff);

	serial_bprintf(buf, "\r\n");

	print_age("Temperature", battery_temp.last_success);

	serial_bprintf(buf, "Onboard temperature: %3.1fC\r\n", battery_temp.onboard_temp / 10.0);
	serial_bprintf(buf, "External temperature 1: %3.1fC, external temperature 2: %3.1fC\r\n",
			battery_temp.external_temp_1 / 10.0, battery_temp.external_temp_2 / 10.0);

	serial_bprintf(buf, "\r\n");

	print_age("SOC", battery_current.last_success);

	serial_bprintf(buf, "Pack SOC: %hu%%, HP SOC: %8.6f%%\r\n",
			battery_soc.stateOfCharge,
			(float) battery_soc.stateOfChargeHp / 1000000);
	serial_bprintf(buf, "Pack SOH: %hu%%\r\n", battery_soc.stateOfHealth);

	serial_bprintf(buf, "\r\n");

	print_age("Safety", battery_safety.last_success);

	serial_bprintf(buf, "Cell fully charged: %4.2fV, pack fully charged: %4.2fV\r\n",
			battery_safety.cell_charged_v / 1000.0,
			(battery_safety.cell_charged_v / 1000.0) * battery_config.cell_count);
	serial_bprintf(buf, "Cell fully discharged: %4.2fV, pack fully discharged: %4.2fV\r\n",
			battery_safety.cell_discharged_v / 1000.0,
			(battery_safety.cell_discharged_v / 1000.0) * battery_config.cell_count);

	serial_bprintf(buf, "\r\n");

	print_age("BMS version", bms_version.last_success);

	serial_bprintf(buf,
			"Hardware version %hhu, hardware changes version %hhu\r\n",
			bms_version.hw_ver.hw_version, bms_version.hw_ver.hw_ch_version);
	serial_bprintf(buf,
			"Public release firmware version %hhu, BPT %hhx BCS %hhx\r\n",
			bms_version.fw_ver.fw_version, bms_version.fw_ver.bpt, bms_version.fw_ver.bcs);
	serial_bprintf(buf, "Internal firmware version %hu\r\n", bms_version.int_fw_ver);
	serial_bprintf(buf, "Bootloader version %hhu, profile version %hhu\r\n",
			bms_version.loader_ver.booloader_ver,
			bms_version.loader_ver.profile_ver);

	uint8_t *s = bms_version.serial_num;
	serial_bprintf(buf,
			"Product serial number %hhx-%hhx-%hhx-%hhx-%hhx-%hhx-%hhx-%hhx-%hhx-%hhx-%hhx-%hhx\r\n",
			s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7], s[8], s[9], s[10], s[11]);

	serial_bprintf(buf, "\r\n");

	serial_bprintf(buf, "Uptime: %us\r\n", millis() / 1000);
	serial_bprintf(buf, "Loop duration: %hums\r\n", loop_duration);
	serial_bprintf(buf, "Free memory: %d\r\n", free_memory);

	serial_bprintf(buf, "\r\n");

}

void send_si_data() {

	// Send data to the Sunny Island
	send_si_safety_frame(&battery_config, &battery_safety, &battery_current, &onsend_callback);
	send_si_soc_frame(&battery_soc, &onsend_callback);
	send_si_sensor_frame(&battery_voltage, &battery_current, &battery_temp, &onsend_callback);
	send_si_faults(&battery_config, &battery_voltage, &battery_current, &battery_safety, &onsend_callback);
	send_si_id_frame(&onsend_callback);
	send_si_system_frame(&bms_version, battery_config.capacity, &onsend_callback);

}

void send_si_fault_data() {

	send_si_faults(&battery_config, &battery_voltage, &battery_current, &battery_safety, &onsend_callback);

}

void send_telemetry_data() {

	send_tiny_telemetry_data(&battery_config, &battery_voltage, &onsend_callback);
	send_system_telemtry_data(loop_duration, free_memory);

}

void update_system_data(unsigned long loop_start_time) {

	loop_duration = millis() - loop_start_time;
	free_memory = get_free_memory();

}

void loop() {

	unsigned long loop_start_time = millis();

	if (next_process_time <= loop_start_time) {

		load_battery_data();

		if (battery_config.cell_count > 0) {

			print_data();
			send_si_data();
			send_telemetry_data();

		} else {

			send_si_fault_data();

		}

		reset_sampling_data();
		update_system_data(loop_start_time);

		next_process_time = loop_start_time + LOOP_PROCESS_INTERVAL;

	} else if (next_sampling_time <= loop_start_time) {

		load_battery_sampling_data();

		next_sampling_time = loop_start_time + LOOP_SAMPLING_INTERVAL;

	}

}
