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
	init_sunnyisland();

	// Allow the hardware to sort itself out
	delay(2000);
	serial_bprintf(buf, "Init OK\r\n");
}

void loop() {

	uint8_t k;
	char tmp[10];

	// Don't proceed if can't determine cell count
	if (load_battery_config(&battery_config) < 0) {
		delay(1000);
		return;
	} else {
		serial_bprintf(buf, "%u configured cells\r\n",
				battery_config.cell_count);
		serial_bprintf(buf, "capacity %uAh\r\n", battery_config.capacity / 100);
	}

	send_name_frame();
	send_id_frame(battery_config.capacity);

	if (load_battery_voltage(&battery_config, &battery_voltage) > 0
			|| battery_voltage.last_success < millis() - 60000) {

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

		serial_printf("Min cell voltage: %sV\r\n",
				dtostrf(battery_voltage.min_cell_voltage / 1000.0, 2, 2, tmp));
		serial_printf("Max cell voltage: %sV\r\n",
				dtostrf(battery_voltage.max_cell_voltage / 1000.0, 2, 2, tmp));

		send_voltage_frame(&battery_voltage);

		serial_bprintf(buf, "\r\n");

	}

	if (load_battery_current(&battery_current) > 0
			|| battery_voltage.last_success < millis() - 60000) {

		serial_bprintf(buf, "Pack current: %sA\r\n",
				dtostrf(battery_current.pack_current, 2, 2, tmp));
		serial_bprintf(buf, "Max discharge current: %uA\r\n",
				battery_current.max_discharge_current);
		serial_bprintf(buf, "Max charge current: %uA\r\n",
				battery_current.max_charge_current);

		serial_bprintf(buf, "\r\n");

	}

	if (load_battery_soc(&battery_soc) > 0
			|| battery_soc.last_success < millis() - 60000) {

		serial_bprintf(buf, "Pack SOC: %u%%\r\n",
				battery_soc.stateOfCharge);

		send_soc_frame(&battery_soc);

		serial_bprintf(buf, "\r\n");

	}

	delay(POLL_INTERVAL);

}
