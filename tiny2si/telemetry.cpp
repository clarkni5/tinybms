#include "telemetry.h"
#include "esp8266.h"
#include "util.h"

char wifi_avail = 0;

void init_telemetry() {

	wifi_avail = init_wifi();
	if (wifi_avail) {
		serial_bprintf(buf, "Wifi initialized\r\n");
	} else {
		serial_bprintf(buf, "Wifi not available\r\n");
	}

}

void print_frame(uint32_t id, uint8_t frame[], uint8_t len) {

	serial_bprintf(buf, "Callback message ID: %lX, len: %hhu, data: ", id, len);
	for (int i = 0; i < len; i++) {
		serial_bprintf(buf, "0x%02hhX ", frame[i]);
	}
	serial_bprintf(buf, "\r\n");

}

void send_wifi_frame(uint32_t id, uint8_t frame[], uint8_t len) {
	uint8_t *datagram = malloc(4 + len);
	memcpy(datagram, &id, 4);
	memcpy(&datagram[4], frame, len);

	for(int i = 0; i < 10; i++) {
		wifi_send_data(datagram, 4 + len);
	}

	free(datagram);
}

void onsend_callback(uint32_t id, uint8_t frame[], uint8_t len) {

	print_frame(id, frame, len);

	if (wifi_avail) {
		send_wifi_frame(id, frame, len);
	}

}

uint8_t* make_voltage_frame(uint16_t pack_voltage, uint16_t min_cell_voltage,
		uint16_t max_cell_voltage, uint16_t cell_count) {

	uint16_t frame[] = {
			leWord(pack_voltage),
			leWord(min_cell_voltage),
			leWord(max_cell_voltage),
			leWord(cell_count)
	};

	return array_dup((uint8_t*) frame, sizeof(frame));

}

void send_voltage_frame(Battery_config *battery_config, Battery_voltage *voltage,
		void (*onsend_callback)(uint32_t id, uint8_t frame[], uint8_t len)) {

	uint8_t *frame = make_voltage_frame(
			(uint16_t) voltage->pack_voltage.fvoltage * 100,
			voltage->min_cell_voltage,
			voltage->max_cell_voltage,
			battery_config->cell_count
		);
	(*onsend_callback)(0x900, frame, 8);
	free(frame);

}

uint8_t* make_system1_frame(uint32_t uptime, uint16_t loop_duration, uint16_t free_memory) {

	uint16_t frame[] = {
		uptime & 0xffff,  // low bytes (least significant)
		(uptime >> 16) & 0xffff,  // high bytes
		loop_duration,
		free_memory
	};

	return array_dup((uint8_t*) frame, sizeof(frame));

}

uint8_t* make_system2_frame(uint16_t free_memory) {

	uint16_t frame[] = {
		free_memory
	};

	return array_dup((uint8_t*) frame, sizeof(frame));

}

void send_tiny_telemetry_data(Battery_config *battery_config, Battery_voltage *voltage,
		void (*onsend_callback)(uint32_t id, uint8_t frame[], uint8_t len)) {

	send_voltage_frame(battery_config, voltage, onsend_callback);

}

void send_system_telemtry_data(uint16_t loop_duration, uint16_t free_memory) {

	uint8_t *frame;

	// System frame 1
	frame = make_system1_frame(
			millis(),
			loop_duration,
			free_memory
		);
	onsend_callback(0x990, frame, 8);
	free(frame);

	// System frame 2
	frame = make_system2_frame(free_memory);
	onsend_callback(0x991, frame, 2);
	free(frame);

}


/*
 * Frame
 * -----
 * 900    Pack voltage | Min cell voltage | Max cell voltage | Cell count
 * 901    Cell 1 voltage | Cell 2 voltage | Cell 3 voltage | Cell 4 voltage
 * 902    Cell 5 V | Cell 6 V | Cell 7 V | Cell 8 V
 * 903    Cell 9 V | Cell 10 V | Cell 11 V | Cell 12 V
 * 904    Cell 13 V | Cell 14 V | Cell 15 V | Cell 16 V
 * 905    Pack charge voltage | Pack discharge voltage | Cell charge voltage | Cell discharge voltage
 *
 * 906    Pack current | sampling min | sampling max | sampling count
 * 907    Current charge limit | Current discharge limit
 *
 * 908    SOC | SOC HP | SOH | Capacity
 *
 * 909    Onboard temp | external temp 1 | external temp 2
 *
 * estimated time left
 * bms online status
 * balancing bits
 *
 * Faults
 *
 * 990    uptime | process loop duration | free memory
 *
 */
