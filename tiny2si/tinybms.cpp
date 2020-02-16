#include "tinybms.h"

AltSoftSerial *softSerial;
ModbusMaster *modbus;

void init_tinybms() {

  softSerial = new AltSoftSerial(MODBUS_RX_PIN, MODBUS_TX_PIN);
  modbus = new ModbusMaster();

  // Init the modbus interface
  softSerial->begin(MODBUS_BAUD);
  modbus->begin(TINYBMS_DEVICE_ID, *softSerial);

}

int readRegistersWithRetry(uint8_t idx, uint8_t count, uint16_t *dest, uint8_t retrcnt) {

  do {

    if(modbus->readHoldingRegisters(idx, count) == modbus->ku8MBSuccess) {
      uint8_t j = 0;
      for (j = 0; j < count; j++) {
        dest[j] = modbus->getResponseBuffer(j);
      }
      break;
    }

    retrcnt--;
    delay(MODBUS_RETRY_INTERVAL);

  }
  while(retrcnt > 0);

  return retrcnt-1;
}

int load_battery_voltage(struct _battery_voltage *voltage) {

  // Get the cell and pack voltages
  if(readRegistersWithRetry(MAX_CELL_COUNT - voltage->cell_count, voltage->cell_count, voltage->cell_voltage.ivoltage, MODBUS_RETRY_COUNT) >= 0) {
    delay(MODBUS_INTERVAL);

    if(readRegistersWithRetry(36, 2, voltage->pack_voltage.ivoltage, MODBUS_RETRY_COUNT) >= 0) {
      voltage->last_success = millis();
      return 1;
    }

  }

  return -1;

}
