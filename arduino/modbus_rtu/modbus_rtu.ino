/**
 * ModbusRTU library: https://github.com/smarmengol/Modbus-Master-Slave-for-Arduino
 * 
 * Requires a JST 2.0mm 4-pin connector to connect to the TinyBMS.
 * Modbus communication is over RS232.
 * 
 * SoftwareSerial modbus connections:
 * WHITE  > X      - not used (this is 5V coming from the TinyBMS)
 * YELLOW > pin  2 - data input pin (this is the TinyBMS Tx pin)
 * BLACK  > pin  3 - data output pin (this is the TinyBMS Rx pin)
 * RED    > ground - ground reference for I/O pins
 */

#include <ModbusRtu.h>
#include <SoftwareSerial.h>

const unsigned int SERIAL_BAUD = 9600;

const uint8_t MODBUS_RX_PIN = 2;
const uint8_t MODBUS_TX_PIN = 3;
const unsigned int MODBUS_BAUD = 115200;

const uint8_t TINYBMS_DEVICE_ID = 0xAA;

// Create the software serial interface
SoftwareSerial softSerial(MODBUS_RX_PIN, MODBUS_TX_PIN);

// Create the modbus interface
Modbus modbus(0, softSerial, 0);

uint8_t state;
unsigned long wait;
uint16_t responseData[16];

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

/**
 * Send a query to read a continuous block of holding registers.
 */
void queryHoldingRegisters(uint16_t u16ReadAddress, uint16_t u16ReadQty, uint16_t data[]) {
  modbus_t telegram;
  telegram.u8id = TINYBMS_DEVICE_ID;
  telegram.u8fct = 0x03;  // function code for reading holding registers
  telegram.u16RegAdd = u16ReadAddress;  // start address in slave
  telegram.u16CoilsNo = u16ReadQty;  // number of registers to read
  telegram.au16reg = data;  // pointer to data array

  modbus.query(telegram);
}

/**
 * Read a continuous block of holding registers. This will block until the
 * request has been completed.
 * 
 * @return 0 on success, 1 on error
 */
int8_t readHoldingRegisters(uint16_t u16ReadAddress, uint16_t u16ReadQty, uint16_t data[]) {
  modbus_t telegram;
  telegram.u8id = TINYBMS_DEVICE_ID;
  telegram.u8fct = 0x03;  // function code for reading holding registers
  telegram.u16RegAdd = u16ReadAddress;  // start address in slave
  telegram.u16CoilsNo = u16ReadQty;  // number of registers to read
  telegram.au16reg = data;  // pointer to data array

  modbus.query(telegram);

  int8_t status;
  do {
    status = modbus.poll();
  } while(modbus.getState() != COM_IDLE);

  if (isResponseStatusOK(status)) {
    return 0;
  }
  return 1;
}

/**
 * Check the response status for an exception or other indication of an error.
 */
boolean isResponseStatusOK(int8_t status) {
  if (status < 6 || status == NO_REPLY) {
    Serial.print("ERROR The response status is ");
    Serial.println(status);
    return false;
  }
  return true;
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println("Sketch: modbus_rtu");

  softSerial.begin(MODBUS_BAUD);
  
  modbus.setTimeOut(2000);  // milliseconds (default is 1000)
  modbus.start();

  state = 0;
  wait = millis() + 1000;
  
  Serial.println("Init OK");
}

void loop() {
  switch(state) {
  case 0: 
    if (millis() > wait) {
      // Done waiting
      state++;
    }
    break;
  case 1: 
    queryHoldingRegisters(36, 2, responseData);
    state++;
    break;
  case 2:
    // Check for a response
    int8_t status = modbus.poll();
    if (modbus.getState() == COM_IDLE) {
      if (isResponseStatusOK(status)) {
        float packVoltage = floatValue(responseData[0], responseData[1]);
        Serial.print("Pack voltage: ");
        Serial.print(packVoltage);
        Serial.println("V");
      }
      state++;
    }
    break;
  case 3:
    // Get some metrics on how long it takes to get modbus data
    unsigned short sampleSize = 20;
    unsigned int errorCount;
    unsigned long startTime;
    unsigned long elapsedTime;
    unsigned short i;
  
    // Check the speed of different ranges
    unsigned short testValuesA[] = { 1, 2, 4, 8, 16, 24, 32, 40 };
    for (i = 0; i < 8; i++) {
      // Reset counters
      errorCount = 0;
      elapsedTime = 0;
    
      // Run test
      int j;
      for (j = 0; j < sampleSize; j++) {
        startTime = millis();
        errorCount += readHoldingRegisters(8, testValuesA[i], responseData);
        elapsedTime += millis() - startTime;
        delay(100);
      }
  
      // Print results
      Serial.print("Read ");
      Serial.print(testValuesA[i]);
      Serial.print(" values:");
      Serial.print("samples=");
      Serial.print(j);
      Serial.print(", avgTime=");
      Serial.print(elapsedTime / j);
      Serial.print(", errorCount=");
      Serial.print(errorCount);
      Serial.println("");
    }
  
    // Check if reserved registers are slower (0-15 vs 16-31)
    unsigned short testValuesB[] = { 45, 49, 110 };
    for (i = 0; i < 8; i++) {
      // Reset counters
      errorCount = 0;
      elapsedTime = 0;
    
      // Run test
      int j;
      for (j = 0; j < sampleSize; j++) {
        startTime = millis();
        errorCount += readHoldingRegisters(testValuesB[i], 1, responseData);
        elapsedTime += millis() - startTime;
        delay(100);
      }
  
      // Print results
      Serial.print("Read reserved register ");
      Serial.print(testValuesB[i]);
      Serial.print(":");
      Serial.print("samples=");
      Serial.print(j);
      Serial.print(", avgTime=");
      Serial.print(elapsedTime / j);
      Serial.print(", errorCount=");
      Serial.print(errorCount);
      Serial.println("");
    }
  
    // Check if ranges with reserved registers are slower (0-15 vs 16-31)
    unsigned short testValuesC[] = { 0, 16 };
    for (i = 0; i < 8; i++) {
      // Reset counters
      errorCount = 0;
      elapsedTime = 0;
    
      // Run test
      int j;
      for (j = 0; j < sampleSize; j++) {
        startTime = millis();
        errorCount += readHoldingRegisters(testValuesC[i], 16, responseData);
        elapsedTime += millis() - startTime;
        delay(100);
      }
  
      // Print results
      Serial.print("Read 16 values starting at register ");
      Serial.print(testValuesC[i]);
      Serial.print(":");
      Serial.print("samples=");
      Serial.print(j);
      Serial.print(", avgTime=");
      Serial.print(elapsedTime / j);
      Serial.print(", errorCount=");
      Serial.print(errorCount);
      Serial.println("");
    }
  
    state = 0;
    wait = millis() + 10000;
    break;
  }
}
