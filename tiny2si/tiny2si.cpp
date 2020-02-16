#include <Arduino.h>
#include "util.h"
#include "tinybms.h"
#include "sunnyisland.h"

#define SERIAL_BAUD 9600
#define POLL_INTERVAL 10000

Battery_voltage battery_voltage;
char buf[128];

void setup() {

  Serial.begin(SERIAL_BAUD);
  serial_bprintf(buf, "Sketch: modbus_master_sw_serial\n");

  init_tinybms();

  // Allow the hardware to sort itself out
  delay(2000);
  serial_bprintf(buf, "Init OK\n");
}

void loop() {

  uint8_t k;
  char tmp[10];

  // Wake up the TinyBMS
  // Don't proceed if can't determine cell count
  if(readRegistersWithRetry(53, 1, &battery_voltage.cell_count, 10) < 0) {
    delay(MODBUS_INTERVAL);
    return;
  }

  if(load_battery_voltage(&battery_voltage) >= 0 || battery_voltage.last_success < millis() - 60000) {

    serial_bprintf(buf, "%u configured cells\n", battery_voltage.cell_count);

    for (k = 0; k < battery_voltage.cell_count; k++) {
      serial_bprintf(buf, "Cell %hhu voltage: %sV\n", battery_voltage.cell_count - k, dtostrf(battery_voltage.cell_voltage.fvoltage[k] / 10000.0, 2, 2, tmp));
    }

    serial_bprintf("Pack voltage: %sV\n", dtostrf(battery_voltage.pack_voltage.fvoltage, 2, 2, tmp));
//    serial_printf("Pack current: %sA [%x %x]\n", dtostrf(floatValue(&responseData[38]), 2, 2, tmp), responseData[38], responseData[39]);
//    serial_printf("Min cell voltage: %sV\n", dtostrf(responseData[40] / 1000.0, 2, 2, tmp));
//    serial_printf("Max cell voltage: %sV\n\n", dtostrf(responseData[41] / 1000.0, 2, 2, tmp));


  }

  delay(POLL_INTERVAL);

}
