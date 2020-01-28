/**
 * ModbusMaster library: https://github.com/4-20ma/ModbusMaster
 */

#include <ModbusMaster.h>
#include <SoftwareSerial.h>

const int SERIAL_BAUD = 9600;

// Create the modbus interface
ModbusMaster node;

SoftwareSerial softSerial(2, 3);

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
  
  softSerial.begin(115200);
  node.begin(0xAA, softSerial);
  
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
    Serial.println("[ku8MBResponseTimedOut] The entire response was not received within the timeout period.");
  } else if (result == node.ku8MBInvalidCRC) {
    Serial.println("[ku8MBInvalidCRC] The CRC in the response does not match the one calculated.");
  } else {
    Serial.println(result);
  }
  
  delay(5000);
}
