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

const unsigned int MODBUS_BAUD = 115200;
const uint8_t MODBUS_DEVICE_ID = 0xAA;

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

// Create the modbus interface
ModbusMaster node;

SoftwareSerial softSerial(2, 3);  // Rx, Tx

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println("Sketch: modbus_master");
  
  softSerial.begin(MODBUS_BAUD);
  node.begin(MODBUS_DEVICE_ID, softSerial);
  
  Serial.println("Init OK");
}

void loop() {
  uint8_t j, result;
  uint16_t words[60];

  // Populate array with some meaningful data from the TinyBMS
  result = node.readHoldingRegisters(0, 16);  // get 16 values starting at register 0
  if (result == node.ku8MBSuccess) {
    for (j = 0; j < 16; j++) {
      words[j] = node.getResponseBuffer(j);
    }
  }
  result = node.readHoldingRegisters(32, 10);  // get 10 values starting at register 32
  if (result == node.ku8MBSuccess) {
    for (j = 0; j < 10; j++) {
      words[32+j] = node.getResponseBuffer(j);
    }
  }

  if (result == node.ku8MBSuccess) {
    uint8_t k;
    for (k = 0; k < 16; k++) {
      Serial.print("Cell ");
      Serial.print(16-k);
      Serial.print(" voltage: ");
      Serial.print(words[k]/10000.0);
      Serial.println("V");
    }
    
    float packVoltage = floatValue(words[36], words[37]);
    Serial.print("Pack voltage: ");
    Serial.print(packVoltage);
    Serial.println("V");
    
  } else if (result == node.ku8MBResponseTimedOut) {
    Serial.println("ERROR [ku8MBResponseTimedOut] The entire response was not received within the timeout period.");
  } else if (result == node.ku8MBInvalidCRC) {
    Serial.println("ERROR [ku8MBInvalidCRC] The CRC in the response does not match the one calculated.");
  } else {
    Serial.print("ERROR [-] ");
    Serial.println(result);
  }
  
  delay(5000);
}
