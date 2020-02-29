#include <Arduino.h>
#include "util.h"
#include "tinybms.h"
#include "sunnyisland.h"

#define SERIAL_BAUD 9600
#define POLL_INTERVAL 10000

Battery_config battery_config;
Battery_voltage battery_voltage;
Battery_current battery_current;
Battery_soc battery_soc;

char buf[128];

void setup() {

	Serial.begin(SERIAL_BAUD);
	serial_bprintf(buf, "Starting tiny2si\r\n");

	init_tinybms();
//	reset_tinybms();
	init_sunnyisland();

// Allow the hardware to sort itself out
	delay(2000);
	serial_bprintf(buf, "Init OK\r\n");
}

void load_battery_data() {

	load_battery_config(&battery_config);
	load_battery_voltage(&battery_config, &battery_voltage);
	load_battery_current(&battery_current);
	load_battery_soc(&battery_soc);

}

void dump_battery_data() {

	char tmp[10];
	uint8_t k;

	serial_bprintf(buf, "%u configured cells\r\n",
			battery_config.cell_count);
	serial_bprintf(buf, "capacity %uAh\r\n", battery_config.capacity / 100);
	serial_bprintf(buf, "voltage data is %us old\r\n",
			(millis() - battery_voltage.last_success) / 1000);

	for (k = 0; k < battery_config.cell_count; k++) {
		serial_bprintf(buf, "Cell %hhu voltage: %sV\r\n",
				battery_config.cell_count - k,
				dtostrf(battery_voltage.cell_voltage[k] / 10000.0, 2, 2,
						tmp));
	}

	serial_bprintf(buf, "Pack voltage: %sV\r\n",
			dtostrf(battery_voltage.pack_voltage.fvoltage, 2, 2, tmp));

	serial_bprintf(buf, "Min cell voltage: %sV\r\n",
			dtostrf(battery_voltage.min_cell_voltage / 1000.0, 2, 2, tmp));
	serial_bprintf(buf, "Max cell voltage: %sV\r\n",
			dtostrf(battery_voltage.max_cell_voltage / 1000.0, 2, 2, tmp));

	send_voltage_frame(&battery_voltage);

	serial_bprintf(buf, "\r\n");

	serial_bprintf(buf, "Pack current: %sA\r\n",
			dtostrf(battery_current.pack_current, 2, 2, tmp));
	serial_bprintf(buf, "Max discharge current: %uA\r\n",
			battery_current.max_discharge_current);
	serial_bprintf(buf, "Max charge current: %uA\r\n",
			battery_current.max_charge_current);

	serial_bprintf(buf, "\r\n");

	serial_bprintf(buf, "Pack SOC: %u%%\r\n", battery_soc.stateOfCharge);
	serial_bprintf(buf, "Pack SOC hp: %lu%%\r\n", battery_soc.stateOfChargeHp);


	serial_bprintf(buf, "\r\n");

}

void send_battery_data() {

	send_name_frame();
	send_id_frame(battery_config.capacity);
	send_soc_frame(&battery_soc);

}

void send_battery_alerts() {

}

void loop() {


	load_battery_data();

	if(battery_config.cell_count > 0) {

		dump_battery_data();
		send_battery_data();
		send_battery_alerts();

	}

	serial_bprintf(buf, "Free memory: %u\r\n", freeMemory());

	delay(POLL_INTERVAL);

}
