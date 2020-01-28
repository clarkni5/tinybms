/**
 * ModbusRTU library: https://github.com/smarmengol/Modbus-Master-Slave-for-Arduino
 */

#include <ModbusRtu.h>
#include <SoftwareSerial.h>

const int SERIAL_BAUD = 9600;

/**
 * Convert two 16-bit words into a 32-bit float.
 * 
 * @return float value
 */
float floatValue(uint16_t word1, uint16_t word2) {
  uint16_t words[2] = { word1, word2 };
  float value;
  memcpy(&value, words, 4);
  return value;
}

// Create the modbus interface
Modbus master(0);
modbus_t telegram;

SoftwareSerial softSerial(2, 3);

unsigned long u32wait;
uint8_t u8state;
uint16_t au16data[16];

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println("Sketch: modbus_rtu");
  
  master.begin(&softSerial, 115200);
  master.setTimeOut(2000);  // milliseconds
  
  u32wait = millis() + 1000;
  u8state = 0;
  
  Serial.println("Init OK");
}

void loop() {
  switch( u8state ) {
  case 0: 
    if (millis() > u32wait) u8state++;  // wait state
    break;
  case 1: 
    telegram.u8id = 0xAA;  // slave address
    telegram.u8fct = 3;  // function code
    telegram.u16RegAdd = 36;  // start address in slave
    telegram.u16CoilsNo = 2;  // number of registers to read
    telegram.au16reg = au16data;  // pointer to data array

    master.query(telegram);
    
    u8state++;
    break;
  case 2:
    master.poll();  // check incoming messages
    if (master.getState() == COM_IDLE) {
      u8state = 0;
      u32wait = millis() + 2000; 
      
      float packVoltage = floatValue(au16data[0], au16data[1]);
      Serial.print("Pack voltage: ");
      Serial.print(packVoltage);
      Serial.println("V");

      delay(5000);
    }
    break;
  }
}
