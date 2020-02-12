/**
 * ModbusMaster library: https://github.com/4-20ma/ModbusMaster
 * 
 * Requires a JST 2.0mm 4-pin connector to connect to the TinyBMS.
 * Modbus communication is over RS232.
 * 
 * SoftwareSerial modbus connections:
 * WHITE  > X      - not used (this is 5V coming from the TinyBMS)
 * YELLOW > pin  0 - data input pin (this is the TinyBMS Tx pin)
 * BLACK  > pin  1 - data output pin (this is the TinyBMS Rx pin)
 * RED    > ground - ground reference for I/O pins
 * 
 * Requires an ethernet shield to send output over the network to 
 * an MQTT broker.
 */

#include <Ethernet.h>
#include <ModbusMaster.h>
#include <PubSubClient.h>
#include <SPI.h>

const unsigned long SERIAL_BAUD = 9600;

const long MODBUS_BAUD = 115200;

const uint8_t TINYBMS_DEVICE_ID = 0xAA;

// Create the modbus interface
ModbusMaster modbus;

// Update the server IP address below
IPAddress server(192, 168, X, X);
const byte mac[] = { 0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
const char* topic = "tinybms/modbus/output";

EthernetClient ethClient;
PubSubClient client(ethClient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      // Connected
    } else {
      // Failed to connect
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void print(String message) {
  if (!client.connected()) {
    reconnect();
  }
  if (client.connected()) {
    // Publish message
    client.publish(topic, message.c_str());
  }
}

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
  // Init the modbus interface
  Serial.begin(MODBUS_BAUD);
  modbus.begin(TINYBMS_DEVICE_ID, Serial);

  // Init the MQTT interface
  client.setServer(server, 1883);
  Ethernet.begin(mac);
  
  // Allow the hardware to sort itself out
  delay(2000);

  print("Init OK");
}

void loop() {
  // Wake up the TinyBMS
  modbus.readHoldingRegisters(0, 1);

  // Get a dump of all the registers...

  uint8_t result;
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
      String message = "Register " + String(registersA[i]) + ": ";
      message += "HEX=" + String(value, HEX) + ", ";
      message += "UINT_16=" + String(value);
      print(message);
    } else {
      print("Error reading register " + String(registersA[i]));
    }
  }

  // Get INT_16 values
  unsigned short registersB[] = {42, 43, 48, 319, 320};
  n = sizeof(registersB) / sizeof(registersB[0]);
  for (i = 0; i < n; i++) {
    result = modbus.readHoldingRegisters(registersB[i], 1);
    if (result == modbus.ku8MBSuccess) {
      int16_t value = modbus.getResponseBuffer(0);
      String message = "Register " + String(registersB[i]) + ": ";
      message += "HEX=" + String(value, HEX) + ", ";
      message += "INT_16=" + String(value);
      print(message);
    } else {
      print("Error reading register " + String(registersB[i]));
    }
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
      String message = "Register " + String(registersC[i]) + ": ";
      message += "HEX=" + String(word1, HEX) + "_" + String(word2, HEX) + ", ";
      message += "UINT_32=" + String(value);
      print(message);
    } else {
      print("Error reading register " + String(registersC[i]));
    }
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
      String message = "Register " + String(registersD[i]) + ": ";
      message += "HEX=" + String(word1, HEX) + "_" + String(word2, HEX) + ", ";
      message += "FLOAT=" + String(value);
      print(message);
    } else {
      print("Error reading register " + String(registersD[i]));
    }
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
      String message = "Register " + String(registersE[i]) + ": ";
      message += "HEX=" + String(value, HEX) + ", ";
      message += "INT_8 LSB=" + String(valueLsb) + ", ";
      message += "INT_8 MSB=" + String(valueMsb);
      print(message);
    } else {
      print("Error reading register " + String(registersE[i]));
    }
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
      String message = "Register " + String(registersF[i]) + ": ";
      message += "HEX=" + String(value, HEX) + ", ";
      message += "UINT_8 LSB=" + String(valueLsb) + ", ";
      message += "UINT_8 MSB=" + String(valueMsb);
      print(message);
    } else {
      print("Error reading register " + String(registersF[i]));
    }
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
      String message = "Register " + String(registersG[i]) + ": ";
      message += "HEX=" + String(value, HEX);
      print(message);
    } else {
      print("Error reading register " + String(registersG[i]));
    }
  }

  // Get some metrics on how long it takes to get modbus data
  unsigned int sampleSize = 50;
  unsigned int errorCount;
  unsigned int crcErrorCount;
  unsigned int timeoutErrorCount;
  unsigned long startTime;
  unsigned long elapsedTime;

  // Check the performance when reading a range of registers
  unsigned short testValuesA[] = {1, 2, 4, 8, 16, 24, 32, 40, 125};
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
    }

    // Print results
    String message = "Read " + String(testValuesA[i]) + ":";
    message += "n=" + String(j) + ",";
    message += "avg=" + String(elapsedTime / j) + "ms,";
    message += "err=" + String(errorCount) + ",";
    message += "crc=" + String(crcErrorCount) + ",";
    message += "tout=" + String(timeoutErrorCount);
    print(message);
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
    }

    // Print results
    String message = String();
    message += "R1 " + String(testValuesB[i]) + ":";
    message += "n=" + String(j) + ",";
    message += "avg=" + String(elapsedTime / j) + "ms,";
    message += "err=" + String(errorCount) + ",";
    message += "crc=" + String(crcErrorCount) + ",";
    message += "tout=" + String(timeoutErrorCount);
    print(message);
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
    }

    // Print results
    String message = String();
    message += "R2 " + String(testValuesC[i]) + ":";
    message += "n=" + String(j) + ",";
    message += "avg=" + String(elapsedTime / j) + "ms,";
    message += "err=" + String(errorCount) + ",";
    message += "crc=" + String(crcErrorCount) + ",";
    message += "tout=" + String(timeoutErrorCount);
    print(message);
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
    String message = "Delay" + String(testValuesD[i]) + ":";
    message += "n=" + String(j) + ",";
    message += "avg=" + String(elapsedTime / j) + "ms,";
    message += "err=" + String(errorCount) + ",";
    message += "crc=" + String(crcErrorCount) + ",";
    message += "tout=" + String(timeoutErrorCount);
    print(message);
  }

  print("DONE");

  delay(30000);
}
