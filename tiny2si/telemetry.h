#ifndef TELEMETRY_H
#define TELEMETRY_H

#include "tinybms.h"

void init_telemetry();

void onsend_callback(uint32_t id, uint8_t frame[], uint8_t len);

void send_tiny_telemetry_data(Battery_config *battery_config, Battery_voltage *voltage,
		void (*onsend_callback)(uint32_t id, uint8_t frame[], uint8_t len));

void send_system_telemtry_data(uint16_t loop_duration, uint16_t free_memory);

#endif

