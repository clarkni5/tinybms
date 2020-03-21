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
//#include <SoftwareSerial.h>
#include <AltSoftSerial.h>
#include <stdarg.h>

#define SERIAL_BAUD 9600

#define MODBUS_RX_PIN 8
#define MODBUS_TX_PIN 9
#define MODBUS_BAUD 115200

#define TINYBMS_DEVICE_ID 0xAA
#define MODBUS_INTERVAL 100

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
  //new SoftwareSerial(MODBUS_RX_PIN, MODBUS_TX_PIN);
  modbus = new ModbusMaster();
  
//  pinMode(MODBUS_RX_PIN, INPUT);
  //pinMode(MODBUS_TX_PIN, OUTPUT);

  // Init the modbus interface
  softSerial->begin(MODBUS_BAUD);
  modbus->begin(TINYBMS_DEVICE_ID, *softSerial);

  // Allow the hardware to sort itself out
  delay(2000);

  serial_printf("Init OK\n");
}

void loop() {
  
  // Wake up the TinyBMS
  modbus->readHoldingRegisters(0, 1);
  
  uint8_t j, result;
  uint16_t responseData[60];

  // Populate array with some meaningful data from the TinyBMS
  result = modbus->readHoldingRegisters(0, 16);  // get 16 values starting at register 0
  if (result == modbus->ku8MBSuccess) {
    for (j = 0; j < 16; j++) {
      responseData[j] = modbus->getResponseBuffer(j);
      delay(MODBUS_INTERVAL);
    }
  } else {
    serial_printf("ERROR %hhu\n", result);
  }
  result = modbus->readHoldingRegisters(32, 10);  // get 10 values starting at register 32
  if (result == modbus->ku8MBSuccess) {
    for (j = 0; j < 10; j++) {
      responseData[32 + j] = modbus->getResponseBuffer(j);
      delay(MODBUS_INTERVAL);
    }
  } else {
    serial_printf("ERROR %hhu\n", result);  
  }

  if (result == modbus->ku8MBSuccess) {
    uint8_t k;
    char volts[10];
    for (k = 0; k < 16; k++) {
      dtostrf(responseData[k] / 10000.0, 4, 6, volts);
      serial_printf("Cell %hhu voltage: %sV\n", 16 - k, volts);
    }
    
    float packVoltage = floatValue(responseData+36);
    dtostrf(packVoltage, 4, 6, volts);
    serial_printf("Pack voltage: %sV\n", voltage);
  } else if (result == modbus->ku8MBResponseTimedOut) {
    serial_printf("ERROR [ku8MBResponseTimedOut] The entire response was not received within the timeout period.\n");
  } else if (result == modbus->ku8MBInvalidCRC) {
    serial_printf("ERROR [ku8MBInvalidCRC] The CRC in the response does not match the one calculated.\n");
  } else {
    serial_printf("ERROR %hhu\n", result);
  }

  unsigned short REQUEST_DELAY = 10;
  delay(REQUEST_DELAY);

  // Get a dump of all the registers...

/*

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
    result = modbus->readHoldingRegisters(registersA[i], 1);
    if (result == modbus->ku8MBSuccess) {
      uint16_t value = modbus->getResponseBuffer(0);
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
    result = modbus->readHoldingRegisters(registersB[i], 1);
    if (result == modbus->ku8MBSuccess) {
      int16_t value = modbus->getResponseBuffer(0);
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
    result = modbus->readHoldingRegisters(registersC[i], 2);
    if (result == modbus->ku8MBSuccess) {
      uint16_t word1 = modbus->getResponseBuffer(0);
      uint16_t word2 = modbus->getResponseBuffer(1);
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
      serial_printf("Error reading register %hu\n", registersC[i]);
    }
    delay(MODBUS_INTERVAL);
  }

  // Get  FLOAT values
  unsigned short registersD[] = {36, 38};
  n = sizeof(registersD) / sizeof(registersD[0]);
  for (i = 0; i < n; i++) {
    result = modbus->readHoldingRegisters(registersD[i], 2);
    if (result == modbus->ku8MBSuccess) {
      uint16_t words[] = { modbus->getResponseBuffer(0), modbus->getResponseBuffer(1) };
      float value = floatValue(words);
      serial_printf("Register %hu : HEX=%x %x, FLOAT=%f\n",registersD[i], words[0], words[2], value);
    } else {
      serial_printf("Error reading register %u\n", registersD[i]);
    }
    delay(MODBUS_INTERVAL);
  }

  // Get double INT_8 values
  unsigned short registersE[] = {113};
  n = sizeof(registersE) / sizeof(registersE[0]);
  for (i = 0; i < n; i++) {
    result = modbus->readHoldingRegisters(registersE[i], 1);
    if (result == modbus->ku8MBSuccess) {
      uint16_t value = modbus->getResponseBuffer(0);
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
    result = modbus->readHoldingRegisters(registersF[i], 1);
    if (result == modbus->ku8MBSuccess) {
      uint16_t value = modbus->getResponseBuffer(0);
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
    result = modbus->readHoldingRegisters(registersG[i], 1);
    if (result == modbus->ku8MBSuccess) {
      uint16_t value = modbus->getResponseBuffer(0);
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
      result = modbus->readHoldingRegisters(8, testValuesA[i]);
      if (result != modbus->ku8MBSuccess) {
        errorCount++;
      }
      if (result == modbus->ku8MBInvalidCRC) {
        crcErrorCount++;
      } else if (result == modbus->ku8MBResponseTimedOut) {
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
      result = modbus->readHoldingRegisters(testValuesB[i], 1);
      if (result != modbus->ku8MBSuccess) {
        errorCount++;
      }
      if (result == modbus->ku8MBInvalidCRC) {
        crcErrorCount++;
      } else if (result == modbus->ku8MBResponseTimedOut) {
        timeoutErrorCount++;
      }
      elapsedTime += millis() - startTime;
      delay(MODBUS_INTERVAL);
    }

    // Print results
    serial_printf("Read reserved register %u:samples=%u, avgTime=%ums, erroCount=%u, timeoutErrorCount=%u\n",testValuesB[i], j, elapsedTime / j, errorCount, crcErrorCount, timeoutErrorCount);
   
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
      result = modbus->readHoldingRegisters(testValuesC[i], 16);
      if (result != modbus->ku8MBSuccess) {
        errorCount++;
      }
      if (result == modbus->ku8MBInvalidCRC) {
        crcErrorCount++;
      } else if (result == modbus->ku8MBResponseTimedOut) {
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
      result = modbus->readHoldingRegisters(8, 1);
      if (result != modbus->ku8MBSuccess) {
        errorCount++;
      }
      if (result == modbus->ku8MBInvalidCRC) {
        crcErrorCount++;
      } else if (result == modbus->ku8MBResponseTimedOut) {
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
  }*/

  Serial.println("DONE");

  delay(10000);
}
