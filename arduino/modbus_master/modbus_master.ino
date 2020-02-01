/**
 * ModbusMaster library: https://github.com/4-20ma/ModbusMaster
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

#include <ModbusMaster.h>
#include <SoftwareSerial.h>

const unsigned int SERIAL_BAUD = 9600;

const uint8_t MODBUS_RX_PIN = 2;
const uint8_t MODBUS_TX_PIN = 3;
const unsigned int MODBUS_BAUD = 115200;

const uint8_t TINYBMS_DEVICE_ID = 0xAA;

// Create the software serial interface
SoftwareSerial softSerial(MODBUS_RX_PIN, MODBUS_TX_PIN);

// Create the modbus interface
ModbusMaster modbus;

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

float floatValue(uint16_t words[]) {
  float value;
  memcpy(&value, words, 4);
  return value;
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println("Sketch: modbus_master");
  
  softSerial.begin(MODBUS_BAUD);
  
  modbus.begin(TINYBMS_DEVICE_ID, softSerial);
  
  Serial.println("Init OK");
}

void loop() {
  uint8_t j, result;
  uint16_t responseData[60];
  uint16_t responseValue;

  // Populate array with some meaningful data from the TinyBMS
  result = modbus.readHoldingRegisters(0, 16);  // get 16 values starting at register 0
  if (result == modbus.ku8MBSuccess) {
    for (j = 0; j < 16; j++) {
      responseData[j] = modbus.getResponseBuffer(j);
    }
  }
  result = modbus.readHoldingRegisters(32, 10);  // get 10 values starting at register 32
  if (result == modbus.ku8MBSuccess) {
    for (j = 0; j < 10; j++) {
      responseData[32+j] = modbus.getResponseBuffer(j);
    }
  }

  if (result == modbus.ku8MBSuccess) {
    uint8_t k;
    for (k = 0; k < 16; k++) {
      Serial.print("Cell ");
      Serial.print(16-k);
      Serial.print(" voltage: ");
      Serial.print(responseData[k]/10000.0);
      Serial.println("V");
    }
    
    float packVoltage = floatValue(responseData[36], responseData[37]);
    Serial.print("Pack voltage: ");
    Serial.print(packVoltage);
    Serial.println("V");
    
  } else if (result == modbus.ku8MBResponseTimedOut) {
    Serial.println("ERROR [ku8MBResponseTimedOut] The entire response was not received within the timeout period.");
  } else if (result == modbus.ku8MBInvalidCRC) {
    Serial.println("ERROR [ku8MBInvalidCRC] The CRC in the response does not match the one calculated.");
  } else {
    Serial.print("ERROR [-] ");
    Serial.println(result);
  }

  
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
      result = modbus.readHoldingRegisters(8, testValuesA[i]);
      if (result != modbus.ku8MBSuccess) {
        errorCount++;
      }
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
    Serial.print(", errorRate=");
    Serial.print(errorCount / j);
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
      result = modbus.readHoldingRegisters(testValuesB[i], 1);
      if (result != modbus.ku8MBSuccess) {
        errorCount++;
      }
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
    Serial.print(", errorRate=");
    Serial.print(errorCount / j);
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
      result = modbus.readHoldingRegisters(testValuesC[i], 16);
      if (result != modbus.ku8MBSuccess) {
        errorCount++;
      }
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
    Serial.print(", errorRate=");
    Serial.print(errorCount / j);
    Serial.println("");
  }

  delay(10000);
}
