/**
 * ModbusMaster library: https://github.com/4-20ma/ModbusMaster
 * 
 * Requires a JST 2.0mm 4-pin connector to connect to the TinyBMS.
 * Modbus communication is over RS232.
 * 
 * AltSofteSerial modbus connections:
 * WHITE  > X      - not used (this is 5V coming from the TinyBMS)
 * YELLOW > pin  8 - data input pin (this is the TinyBMS Tx pin)
 * BLACK  > pin  9 - data output pin (this is the TinyBMS Rx pin)
 * RED    > ground - ground reference for I/O pins
 */

#include <ModbusMaster.h>
#include <AltSoftSerial.h>
#include <stdarg.h>

#define SERIAL_BAUD 9600

#define MODBUS_RX_PIN 8
#define MODBUS_TX_PIN 9
#define MODBUS_BAUD 115200

#define TINYBMS_DEVICE_ID 0xAA
#define MODBUS_INTERVAL 100
#define MODBUS_RETRY_INTERVAL 100
#define POLL_INTERVAL 1000

// Declare handlers, but don't initialize them yet
AltSoftSerial *softSerial;
ModbusMaster *modbus;

#define uint32Value(x, y) (x << 16 | y)
#define floatValue(x) (*(float*)x)
#define int8LsbValue(x) ((uint8_t)x)
#define int8MsbValue(x) ((uint8_t) x >> 8)
#define uint8LsbValue(x) int8LsbValue(x) 
#define uint8MsbValue(x) int8MsbValue(x)

void serial_printf(const char *fmt, ...) {
  char buf[128];
  
  va_list argp;
  va_start(argp, fmt);
  vsprintf(buf, fmt, argp);
  Serial.print(buf); 
  va_end(argp);
}

void setup() {
  
  Serial.begin(SERIAL_BAUD);
  serial_printf("Sketch: modbus_master_sw_serial\n");
  
  softSerial = new AltSoftSerial(MODBUS_RX_PIN, MODBUS_TX_PIN);
  modbus = new ModbusMaster();
  
  // Init the modbus interface
  softSerial->begin(MODBUS_BAUD);
  modbus->begin(TINYBMS_DEVICE_ID, *softSerial);

  // Allow the hardware to sort itself out
  delay(2000);
  serial_printf("Init OK\n");
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
  
 } while(retrcnt > 0);
  
  return retrcnt-1;
}

void loop() {
  
  uint16_t cell_count;
  uint16_t responseData[60];
  float pack_voltage;
  
  // Wake up the TinyBMS
  // Don't proceed if can't determine cell count
  if(readRegistersWithRetry(53, 1, &cell_count, 10) < 0) {
     delay(MODBUS_INTERVAL);
     return; 
  }

  // Get the cell and pack voltages  
  if(readRegistersWithRetry(16 - cell_count, 16, responseData, 5) >= 0) {
    delay(MODBUS_INTERVAL);
    
    if(readRegistersWithRetry(32, 10, &responseData[32], 5) >= 0) {
      delay(MODBUS_INTERVAL);
      
      if(readRegistersWithRetry(36, 2, (uint16_t*)&pack_voltage, 5) >= 0) {      
      
        uint8_t k;
        char tmp[10];
      
        serial_printf("%u configured cells\n", cell_count);
      
        for (k = 0; k < cell_count; k++) {
          serial_printf("Cell %hhu voltage: %sV\n", cell_count - k, dtostrf(responseData[k] / 10000.0, 2, 2, tmp));
        }
    
        serial_printf("Pack voltage: %sV\n", dtostrf(pack_voltage, 2, 2, tmp));
        serial_printf("Pack current: %sA [%x %x]\n", dtostrf(floatValue(&responseData[38]), 2, 2, tmp), responseData[38], responseData[39]);
        serial_printf("Min cell voltage: %sV\n", dtostrf(responseData[40] / 1000.0, 2, 2, tmp));
        serial_printf("Max cell voltage: %sV\n\n", dtostrf(responseData[41] / 1000.0, 2, 2, tmp));
        
      }
      
    }
    
  }
 
  delay(POLL_INTERVAL);

}
