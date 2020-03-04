/**
 * ModbusMaster library: https://github.com/4-20ma/ModbusMaster
 * 
 * Requires a JST 2.0mm 4-pin connector to connect to the TinyBMS.
 * Modbus communication is over RS232.
 * 
 * Serial1 modbus connections (for Arduino Mega):
 * WHITE  > X      - not used (this is 5V coming from the TinyBMS)
 * YELLOW > pin 19 - data input pin (this is the TinyBMS Tx pin)
 * BLACK  > pin 18 - data output pin (this is the TinyBMS Rx pin)
 * RED    > ground - ground reference for I/O pins
 */

#include <ModbusMaster.h>

const unsigned long SERIAL_BAUD = 9600;

const long MODBUS_BAUD = 115200;

const uint8_t TINYBMS_DEVICE_ID = 0xAA;

const uint8_t MODBUS_INTERVAL = 100;

// Create the modbus interface
ModbusMaster modbus;

/**
 * Convert two 16-bit words into a 32-bit unsigned integer.
 * 
 * @return uint32_t value
 */
uint32_t uint32Value(uint16_t word1, uint16_t word2) {
  uint16_t words[2] = {word1, word2};
  uint32_t value;
  memcpy(&value, words, 4);
  return value;
}

/**
 * Convert two 16-bit words into a 32-bit float.
 * 
 * @return float value
 */
float floatValue(uint16_t word1, uint16_t word2) {
  uint16_t words[2] = {word1, word2};
  float value;
  memcpy(&value, words, 4);
  return value;
}

float floatValue(uint16_t words[]) {
  float value;
  memcpy(&value, words, 4);
  return value;
}

int8_t int8LsbValue(uint16_t word1) {
  return (int8_t)word1;
}

int8_t int8MsbValue(uint16_t word1) {
  return (int8_t)(word1 >> 8);
}

uint8_t uint8LsbValue(uint16_t word1) {
  return (uint8_t)word1;
}

uint8_t uint8MsbValue(uint16_t word1) {
  return (uint8_t)(word1 >> 8);
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println("Sketch: modbus_master_hw_serial_mega");

  // Init the modbus interface
  Serial1.begin(MODBUS_BAUD);
  modbus.begin(TINYBMS_DEVICE_ID, Serial1);

  // Allow the hardware to sort itself out
  delay(2000);

  Serial.println("Init OK");
}

void loop() {
  // Wake up the TinyBMS
  modbus.readHoldingRegisters(0, 1);
  
  uint8_t j, result;
  uint16_t responseData[60];

  // Populate array with some meaningful data from the TinyBMS
  result = modbus.readHoldingRegisters(0, 16);  // get 16 values starting at register 0
  if (result == modbus.ku8MBSuccess) {
    for (j = 0; j < 16; j++) {
      responseData[j] = modbus.getResponseBuffer(j);
      delay(MODBUS_INTERVAL);
    }
  } else {
    Serial.print("ERROR ");
    Serial.println(result);
  }
  result = modbus.readHoldingRegisters(32, 10);  // get 10 values starting at register 32
  if (result == modbus.ku8MBSuccess) {
    for (j = 0; j < 10; j++) {
      responseData[32 + j] = modbus.getResponseBuffer(j);
      delay(MODBUS_INTERVAL);
    }
  } else {
    Serial.print("ERROR ");
    Serial.println(result);
  }

  if (result == modbus.ku8MBSuccess) {
    uint8_t k;
    for (k = 0; k < 16; k++) {
      Serial.print("Cell ");
      Serial.print(16 - k);
      Serial.print(" voltage: ");
      Serial.print(responseData[k] / 10000.0);
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
    Serial.print("ERROR ");
    Serial.println(result);
  }

  unsigned short REQUEST_DELAY = 10;
  delay(REQUEST_DELAY);

  // Get a dump of all the registers...

  unsigned short i;
  size_t n;

  // Get UINT_16 values
  unsigned short registersA[] = {
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
      11, 12, 13, 14, 15, 40, 41, 50, 51, 52,
      53, 102, 103, 104, 105, 106, 107, 108, 109, 111,
      112, 300, 301, 303, 304, 306, 307, 308, 315, 316,
      317, 318, 328, 502};
  n = sizeof(registersA) / sizeof(registersA[0]);
  for (i = 0; i < n; i++) {
    result = modbus.readHoldingRegisters(registersA[i], 1);
    if (result == modbus.ku8MBSuccess) {
      uint16_t value = modbus.getResponseBuffer(0);
      Serial.print("Register ");
      Serial.print(registersA[i]);
      Serial.print(": HEX=");
      Serial.print(value, HEX);
      Serial.print(", UINT_16=");
      Serial.println(value);
    } else {
      Serial.print("Error reading register ");
      Serial.println(registersA[i]);
    }
    delay(MODBUS_INTERVAL);
  }

  // Get INT_16 values
  unsigned short registersB[] = {42, 43, 48, 319, 320};
  n = sizeof(registersB) / sizeof(registersB[0]);
  for (i = 0; i < n; i++) {
    result = modbus.readHoldingRegisters(registersB[i], 1);
    if (result == modbus.ku8MBSuccess) {
      int16_t value = modbus.getResponseBuffer(0);
      Serial.print("Register ");
      Serial.print(registersB[i]);
      Serial.print(": HEX=");
      Serial.print(value, HEX);
      Serial.print(", INT_16=");
      Serial.println(value);
    } else {
      Serial.print("Error reading register ");
      Serial.println(registersB[i]);
    }
    delay(MODBUS_INTERVAL);
  }

  // Get UINT_32 values
  unsigned short registersC[] = {32, 34, 46, 116};
  n = sizeof(registersC) / sizeof(registersC[0]);
  for (i = 0; i < n; i++) {
    result = modbus.readHoldingRegisters(registersC[i], 2);
    if (result == modbus.ku8MBSuccess) {
      uint16_t word1 = modbus.getResponseBuffer(0);
      uint16_t word2 = modbus.getResponseBuffer(1);
      uint32_t value = uint32Value(word1, word2);
      Serial.print("Register ");
      Serial.print(registersC[i]);
      Serial.print(": HEX=");
      Serial.print(word1, HEX);
      Serial.print(" ");
      Serial.print(word2, HEX);
      Serial.print(", UINT_32=");
      Serial.println(value);
    } else {
      Serial.print("Error reading register ");
      Serial.println(registersC[i]);
    }
    delay(MODBUS_INTERVAL);
  }

  // Get  FLOAT values
  unsigned short registersD[] = {36, 38};
  n = sizeof(registersD) / sizeof(registersD[0]);
  for (i = 0; i < n; i++) {
    result = modbus.readHoldingRegisters(registersD[i], 2);
    if (result == modbus.ku8MBSuccess) {
      uint16_t word1 = modbus.getResponseBuffer(0);
      uint16_t word2 = modbus.getResponseBuffer(1);
      float value = floatValue(word1, word2);
      Serial.print("Register ");
      Serial.print(registersD[i]);
      Serial.print(": HEX=");
      Serial.print(word1, HEX);
      Serial.print(" ");
      Serial.print(word2, HEX);
      Serial.print(", FLOAT=");
      Serial.println(value);
    } else {
      Serial.print("Error reading register ");
      Serial.println(registersD[i]);
    }
    delay(MODBUS_INTERVAL);
  }

  // Get double INT_8 values
  unsigned short registersE[] = {113};
  n = sizeof(registersE) / sizeof(registersE[0]);
  for (i = 0; i < n; i++) {
    result = modbus.readHoldingRegisters(registersE[i], 1);
    if (result == modbus.ku8MBSuccess) {
      uint16_t value = modbus.getResponseBuffer(0);
      int8_t valueLsb = int8LsbValue(value);
      int8_t valueMsb = int8MsbValue(value);
      Serial.print("Register ");
      Serial.print(registersE[i]);
      Serial.print(": HEX=");
      Serial.print(value, HEX);
      Serial.print(", INT_8 LSB=");
      Serial.print(valueLsb);
      Serial.print(", INT_8 MSB=");
      Serial.println(valueMsb);
    } else {
      Serial.print("Error reading register ");
      Serial.println(registersE[i]);
    }
    delay(MODBUS_INTERVAL);
  }

  // Get double UINT_8 values
  unsigned short registersF[] = {114};
  n = sizeof(registersF) / sizeof(registersF[0]);
  for (i = 0; i < n; i++) {
    result = modbus.readHoldingRegisters(registersF[i], 1);
    if (result == modbus.ku8MBSuccess) {
      uint16_t value = modbus.getResponseBuffer(0);
      int8_t valueLsb = uint8LsbValue(value);
      int8_t valueMsb = uint8MsbValue(value);
      Serial.print("Register ");
      Serial.print(registersF[i]);
      Serial.print(": HEX=");
      Serial.print(value, HEX);
      Serial.print(", UINT_8 LSB=");
      Serial.print(valueLsb);
      Serial.print(", UINT_8 MSB=");
      Serial.println(valueMsb);
    } else {
      Serial.print("Error reading register ");
      Serial.println(registersF[i]);
    }
    delay(MODBUS_INTERVAL);
  }

  // Get HEX values of other interesting registers
  unsigned short registersG[] = {
      200, 201, 202, 203, 204, 205, 206, 207, 296, 297,
      500, 501, 503};
  n = sizeof(registersG) / sizeof(registersG[0]);
  for (i = 0; i < n; i++) {
    result = modbus.readHoldingRegisters(registersG[i], 1);
    if (result == modbus.ku8MBSuccess) {
      uint16_t value = modbus.getResponseBuffer(0);
      Serial.print("Register ");
      Serial.print(registersG[i]);
      Serial.print(": HEX=");
      Serial.println(value, HEX);
    } else {
      Serial.print("Error reading register ");
      Serial.println(registersG[i]);
    }
    delay(MODBUS_INTERVAL);
  }

  // Get some metrics on how long it takes to get modbus data
  unsigned int sampleSize = 50;
  unsigned int errorCount;
  unsigned int crcErrorCount;
  unsigned int timeoutErrorCount;
  unsigned long startTime;
  unsigned long elapsedTime;

  // Check the performance when reading a range of registers
  unsigned short testValuesA[] = {1, 2, 4, 8, 16, 24, 32, 40};
  n = sizeof(testValuesA) / sizeof(testValuesA[0]);
  for (i = 0; i < n; i++) {
    // Reset counters
    errorCount = 0;
    crcErrorCount = 0;
    timeoutErrorCount = 0;
    elapsedTime = 0;

    // Run test
    unsigned int j;
    for (j = 0; j < sampleSize; j++) {
      startTime = millis();
      result = modbus.readHoldingRegisters(8, testValuesA[i]);
      if (result != modbus.ku8MBSuccess) {
        errorCount++;
      }
      if (result == modbus.ku8MBInvalidCRC) {
        crcErrorCount++;
      } else if (result == modbus.ku8MBResponseTimedOut) {
        timeoutErrorCount++;
      }
      elapsedTime += millis() - startTime;
      delay(MODBUS_INTERVAL);
    }

    // Print results
    Serial.print("Read ");
    Serial.print(testValuesA[i]);
    Serial.print(" values:");
    Serial.print("samples=");
    Serial.print(j);
    Serial.print(", avgTime=");
    Serial.print(elapsedTime / j);
    Serial.print("ms, errorCount=");
    Serial.print(errorCount);
    Serial.print(", crcErrorCount=");
    Serial.print(crcErrorCount);
    Serial.print(", timeoutErrorCount=");
    Serial.print(timeoutErrorCount);
    Serial.println("");
  }

  // Check if reserved registers are slower (0-15 vs 16-31)
  unsigned short testValuesB[] = {45, 49, 110};
  n = sizeof(testValuesB) / sizeof(testValuesB[0]);
  for (i = 0; i < n; i++) {
    // Reset counters
    errorCount = 0;
    crcErrorCount = 0;
    timeoutErrorCount = 0;
    elapsedTime = 0;

    // Run test
    unsigned int j;
    for (j = 0; j < sampleSize; j++) {
      startTime = millis();
      result = modbus.readHoldingRegisters(testValuesB[i], 1);
      if (result != modbus.ku8MBSuccess) {
        errorCount++;
      }
      if (result == modbus.ku8MBInvalidCRC) {
        crcErrorCount++;
      } else if (result == modbus.ku8MBResponseTimedOut) {
        timeoutErrorCount++;
      }
      elapsedTime += millis() - startTime;
      delay(MODBUS_INTERVAL);
    }

    // Print results
    Serial.print("Read reserved register ");
    Serial.print(testValuesB[i]);
    Serial.print(":");
    Serial.print("samples=");
    Serial.print(j);
    Serial.print(", avgTime=");
    Serial.print(elapsedTime / j);
    Serial.print("ms, errorCount=");
    Serial.print(errorCount);
    Serial.print(", crcErrorCount=");
    Serial.print(crcErrorCount);
    Serial.print(", timeoutErrorCount=");
    Serial.print(timeoutErrorCount);
    Serial.println("");
  }

  // Check if ranges with reserved registers are slower (0-15 vs 16-31)
  unsigned short testValuesC[] = {0, 16};
  n = sizeof(testValuesC) / sizeof(testValuesC[0]);
  for (i = 0; i < n; i++) {
    // Reset counters
    errorCount = 0;
    crcErrorCount = 0;
    timeoutErrorCount = 0;
    elapsedTime = 0;

    // Run test
    unsigned int j;
    for (j = 0; j < sampleSize; j++) {
      startTime = millis();
      result = modbus.readHoldingRegisters(testValuesC[i], 16);
      if (result != modbus.ku8MBSuccess) {
        errorCount++;
      }
      if (result == modbus.ku8MBInvalidCRC) {
        crcErrorCount++;
      } else if (result == modbus.ku8MBResponseTimedOut) {
        timeoutErrorCount++;
      }
      elapsedTime += millis() - startTime;
      delay(MODBUS_INTERVAL);
    }

    // Print results
    Serial.print("Read 16 values starting at register ");
    Serial.print(testValuesC[i]);
    Serial.print(":");
    Serial.print("samples=");
    Serial.print(j);
    Serial.print(", avgTime=");
    Serial.print(elapsedTime / j);
    Serial.print("ms, errorCount=");
    Serial.print(errorCount);
    Serial.print(", crcErrorCount=");
    Serial.print(crcErrorCount);
    Serial.print(", timeoutErrorCount=");
    Serial.print(timeoutErrorCount);
    Serial.println("");
  }

  // Check the error rate when imposing a delay between messages
  unsigned int testValuesD[] = {10, 20, 50, 100, 200, 400};
  n = sizeof(testValuesD) / sizeof(testValuesD[0]);
  for (i = 0; i < n; i++) {
    // Reset counters
    errorCount = 0;
    crcErrorCount = 0;
    timeoutErrorCount = 0;
    elapsedTime = 0;

    // Run test
    unsigned int j;
    for (j = 0; j < sampleSize; j++) {
      delay(testValuesD[i]);

      startTime = millis();
      result = modbus.readHoldingRegisters(8, 1);
      if (result != modbus.ku8MBSuccess) {
        errorCount++;
      }
      if (result == modbus.ku8MBInvalidCRC) {
        crcErrorCount++;
      } else if (result == modbus.ku8MBResponseTimedOut) {
        timeoutErrorCount++;
      }
      elapsedTime += millis() - startTime;
    }

    // Print results
    Serial.print("Read with an interval of ");
    Serial.print(testValuesD[i]);
    Serial.print("ms:");
    Serial.print("samples=");
    Serial.print(j);
    Serial.print(", avgTime=");
    Serial.print(elapsedTime / j);
    Serial.print("ms, errorCount=");
    Serial.print(errorCount);
    Serial.print(", crcErrorCount=");
    Serial.print(crcErrorCount);
    Serial.print(", timeoutErrorCount=");
    Serial.print(timeoutErrorCount);
    Serial.println("");
  }

  Serial.println("DONE");

  delay(30000);
}
