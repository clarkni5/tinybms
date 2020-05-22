/**
 * Send fake CAN bus data to the SMA Sunny Island.
 * 
 * Required library: https://github.com/Seeed-Studio/CAN_BUS_Shield
 * 
 * MCP2515 breakout board connections:
 * INT > pin  2 - interrupt output pin (not used in this sketch)
 * SCK > pin 13 - clock input pin for SPI interface
 * SI  > pin 11 - data input pin for SPI interface
 * SO  > pin 12 - data output pin for SPI interface
 * CS  > pin 10 - chip select input pin for SPI interface
 * GND > ground - ground reference for logic and I/O pins
 * VCC > 5V     - positive supply for logic and I/O pins
 */

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <mcp_can.h>

#define BIT_FLAG_1 0b00000001
#define BIT_FLAG_2 0b00000100
#define BIT_FLAG_3 0b00010000
#define BIT_FLAG_4 0b01000000

const int SERIAL_BAUD = 9600;

IPAddress ip(192, 168, 1, 99);
byte mac[] = { 0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };

IPAddress server(192, 168, 1, 50);
const char* topic = "tinybms/canbus/test";

// Create the CAN interface
MCP_CAN CAN(10);  // using CS pin 10

EthernetClient ethClient;
PubSubClient client(ethClient);

// The Sunny Island relies heavily on it's own voltage reading.
// Use the actual battery voltage to adjust the tests.
signed long actualBatteryVoltage = 558;

unsigned long finalChargeVoltage;
signed long maxChargeCurrent;
signed long maxDischargeCurrent;
unsigned long finalDischargeVoltage;

unsigned long stateOfCharge;
unsigned long stateOfHealth;
unsigned long stateOfChargeHighPrecision;

signed long batteryVoltage;
signed long batteryCurrent;
signed long batteryTemp;

byte fault[4] = {0, 0, 0, 0};
byte warning[4] = {0, 0, 0, 0};

signed long batteryCapacity;

String bmsName;

unsigned long messageInterval;
unsigned long measurementCycleDelay;

bool sendMessage351;
bool sendMessage355;
bool sendMessage356;
bool sendMessage35A;
bool sendMessage35E;
bool sendMessage35F;

unsigned long startTime;
char currentCommand = 'r';

// Reset the BMS values
void resetBmsValues() {
  finalChargeVoltage = 572;  // 57.2V
  maxChargeCurrent = 99;  // 9.9A
  maxDischargeCurrent = 121;  // 12.1A
  finalDischargeVoltage = 492;  // 49.2V
  
  stateOfCharge = 100;  // 100%
  stateOfHealth = 99;  // 99%
  stateOfChargeHighPrecision = 10000;  // 100.00%
  
  batteryVoltage = 5320;  // 53.20V
  batteryCurrent = 24;  // 2.4A discharging current is positive
  batteryTemp = 241;  // 24.1C

  for (int i = 0; i < 4; i++) {
    fault[i] = 0;
    warning[i] = 0;
  }
  
  batteryCapacity = 1250;  // 1250Ah
  
  bmsName = "TinyBMS";
  
  messageInterval = 210;
  measurementCycleDelay = 740;

  sendMessage351 = true;
  sendMessage355 = true;
  sendMessage356 = true;
  sendMessage35A = true;
  sendMessage35E = true;
  sendMessage35F = true;
}

// Print the current BMS values
void printBmsValues() {
  Serial.println("BMS values");
  
  Serial.print("  finalChargeVoltage: ");
  Serial.println(finalChargeVoltage);

  Serial.print("  maxChargeCurrent: ");
  Serial.println(maxChargeCurrent);

  Serial.print("  maxDischargeCurrent: ");
  Serial.println(maxDischargeCurrent);

  Serial.print("  finalDischargeVoltage: ");
  Serial.println(finalDischargeVoltage);

  Serial.print("  stateOfCharge: ");
  Serial.println(stateOfCharge);

  Serial.print("  stateOfHealth: ");
  Serial.println(stateOfHealth);

  Serial.print("  stateOfChargeHighPrecision: ");
  Serial.println(stateOfChargeHighPrecision);

  Serial.print("  batteryVoltage: ");
  Serial.println(batteryVoltage);

  Serial.print("  batteryCurrent: ");
  Serial.println(batteryCurrent);

  Serial.print("  batteryTemp: ");
  Serial.println(batteryTemp);

  Serial.print("  fault: ");
  Serial.print(fault[0], BIN);
  Serial.print("_");
  Serial.print(fault[1], BIN);
  Serial.print("_");
  Serial.print(fault[2], BIN);
  Serial.print("_");
  Serial.println(fault[3], BIN);

  Serial.print("  warning: ");
  Serial.print(warning[0], BIN);
  Serial.print("_");
  Serial.print(warning[1], BIN);
  Serial.print("_");
  Serial.print(warning[2], BIN);
  Serial.print("_");
  Serial.println(warning[3], BIN);

  Serial.print("  batteryCapacity: ");
  Serial.println(batteryCapacity);

  Serial.print("  bmsName: ");
  Serial.println(bmsName);

  Serial.print("  messageInterval: ");
  Serial.println(messageInterval);

  Serial.print("  measurementCycleDelay: ");
  Serial.println(measurementCycleDelay);
}

// MQTT message callback
void callback(char* topic, byte* payload, unsigned int length) {
  char command = payload[0];
  
  Serial.print("Received command '");
  Serial.print(command);
  Serial.println("'");

  unsigned long elapsedTime;
  
  switch(command) {
    case '.':
      elapsedTime = millis() - startTime;
      Serial.print("Time since command '");
      Serial.print(currentCommand);
      Serial.print("': ");
      Serial.print(elapsedTime);
      Serial.println("ms");
      break;
    case 'a':
      // Stop sending message 0x351
      // Probably a required message
      sendMessage351 = false;
      break;
    case 'b':
      // Stop sending message 0x355
      // Probably a required message
      sendMessage355 = false;
      break;
    case 'c':
      // Stop sending message 0x356
      sendMessage356 = false;
      break;
    case 'd':
      // Stop sending message 0x35A
      // Probably a required message
      sendMessage35A = false;
      break;
    case 'e':
      // Stop sending message 0x35E
      sendMessage35E = false;
      break;
    case 'f':
      // Stop sending message 0x35F
      sendMessage35F = false;
      break;
    case 'g':
      // Set a low battery voltage
      // This value appears to be ignored
      batteryVoltage = 4130;  // 41.30V
      break;
    case 'h':
      // Set a high battery current
      // This value appears to be ignored
      batteryCurrent = 1598;  // 159.8A
      break;
    case 'i':
      // Set a negative battery current
      // Does this value get factored into the allowed charge current?
      batteryCurrent = -102;  // -10.2A
      break;
    case 'j':
      // Set final discharge voltage just below the actual voltage
      // This does not cause a warning
      finalDischargeVoltage = actualBatteryVoltage - 1; // subtract 0.1V
      break;
    case 'k':
      // Set final discharge voltage to the actual voltage
      // This does not cause a warning
      finalDischargeVoltage = actualBatteryVoltage;
      break;
    case 'l':
      // Set final discharge voltage above the actual voltage
      // - Triggers F213 BatVtgLow
      // - Goes into standby mode
      // - Will auto-start when voltage goes above threshold
      finalDischargeVoltage = actualBatteryVoltage + 1; // add 0.1V
      break;
    case 'm':
      // Set the final charge voltage just above the current voltage
      // Expect nothing to happen
      finalChargeVoltage = actualBatteryVoltage + 1; // add 0.1V
      break;
    case 'n':
      // Set the final charge voltage to the current voltage
      // Does it continue charging?
      finalChargeVoltage = actualBatteryVoltage;
      break;
    case 'o':
      // Set the final charge voltage just below the current voltage
      // It should stop charging
      finalChargeVoltage = actualBatteryVoltage - 1; // subtract 0.1V
      break;
    case 'p':
      // Set a lower SOC to start charging the battery
      // The high precision SOC is visible from menu 120.01#BatSoc
      // Does this start the charging process?
      stateOfCharge = 52;  // 52%
      stateOfChargeHighPrecision = 5230;  // 52.30%
      break;
    case 'q':
      // Set a very low SOC
      // The high precision SOC is visible from menu 120.01#BatSoc
      // This does not cause a warning
      stateOfCharge = 2;  // 2%
      stateOfChargeHighPrecision = 230;  // 2.30%
      break;
    case 'r': // reset
      // If charging, this should stop the charing process
      resetBmsValues();
      break;
    case 's':
      // Set the max discharge current to 0.5A
      // With a 0.4A load applied, does this cause a warning? No
      maxDischargeCurrent = 5;  // 0.5A
      //batteryCurrent = 120;  // 12.0A
      break;
    case 't':
      // Set the max discharge current to 0.4A
      // With a 0.4A load applied, does this cause a warning? No
      maxDischargeCurrent = 4;  // 0.4A
      break;
    case 'u':
      // Set the max discharge current to 0.2A
      // With a 0.4A load applied, does this cause a fault? No
      maxDischargeCurrent = 2;  // 0.2A
      break;
    case 'v':
      // Set a modest max charge current
      // Expect 7.2A @ 57.2V = 412W
      // Or maybe 7.2A @ 56.2V = 405W
      maxChargeCurrent = 72;  // 7.2A
      break;
    case 'w':
      // Set a higher max charge current
      // Expect 20.1A @ 57.2V = 1150W
      // Or maybe 20.1A @ 56.2V = 1130W
      maxChargeCurrent = 201;  // 20.1A
      break;
    case 'x':
      // High current, above charge current limit
      // No longer valid
      maxChargeCurrent = 99;  // 9.9A
      batteryCurrent = -100;  // -10.0A
      break;
    case 'y':
      // Low temperature
      // This does not cause a warning or fault
      batteryTemp = -11;  // -1.1C
      break;
    case 'z':
      // High temperature
      // This does not cause a warning or fault
      batteryTemp = 601;  // 60.1C
      break;
    case 'A':
      //  General fault
      fault[0] = fault[0] | BIT_FLAG_1;
      break;
    case 'B':
      // Battery high voltage
      fault[0] = fault[0] | BIT_FLAG_2;
      break;
    case 'C':
      // Battery low voltage
      fault[0] = fault[0] | BIT_FLAG_3;
      break;
    case 'D':
      // Battery high temp
      fault[0] = fault[0] | BIT_FLAG_4;
      break;
    case 'E':
      // Battery low temp
      fault[1] = fault[1] | BIT_FLAG_1;
      break;
    case 'F':
      // Battery high temp charge
      fault[1] = fault[1] | BIT_FLAG_2;
      break;
    case 'G':
      // Battery low temp charge
      fault[1] = fault[1] | BIT_FLAG_3;
      break;
    case 'H':
      // Battery high current discharge
      fault[1] = fault[1] | BIT_FLAG_4;
      break;
    case 'I':
      // Battery high current charge
      fault[2] = fault[2] | BIT_FLAG_1;
      break;
    case 'J':
      // Short circuit
      fault[2] = fault[2] | BIT_FLAG_2;
      break;
    case 'K':
      // BMS fault
      fault[2] = fault[2] | BIT_FLAG_3;
      break;
    case 'L':
      // Cell imbalance
      fault[2] = fault[2] | BIT_FLAG_4;
      break;
    case 'M':
      // Unknown
      fault[3] = fault[3] | BIT_FLAG_1;
      break;
    case 'N':
      // Unknown
      fault[3] = fault[3] | BIT_FLAG_2;
      break;
    case 'O':
      // Unknown
      fault[3] = fault[3] | BIT_FLAG_3;
      break;
    case 'P':
      // Unknown
      fault[3] = fault[3] | BIT_FLAG_4;
      break;
    case 'Q':
      //  General warning
      warning[0] = warning[0] | BIT_FLAG_1;
      break;
    case 'R':
      // Battery high voltage
      warning[0] = warning[0] | BIT_FLAG_2;
      break;
    case 'S':
      // Battery low voltage
      warning[0] = warning[0] | BIT_FLAG_3;
      break;
    case 'T':
      // Battery high temp
      warning[0] = warning[0] | BIT_FLAG_4;
      break;
    case 'U':
      // Battery low temp
      warning[1] = warning[1] | BIT_FLAG_1;
      break;
    case 'V':
      // Battery high temp charge
      warning[1] = warning[1] | BIT_FLAG_2;
      break;
    case 'W':
      // Battery low temp charge
      warning[1] = warning[1] | BIT_FLAG_3;
      break;
    case 'X':
      // Battery high current discharge
      warning[1] = warning[1] | BIT_FLAG_4;
      break;
    case 'Y':
      // Battery high current charge
      warning[2] = warning[2] | BIT_FLAG_1;
      break;
    case 'Z':
      // Short circuit
      warning[2] = warning[2] | BIT_FLAG_2;
      break;
    case '1':
      // BMS fault
      warning[2] = warning[2] | BIT_FLAG_3;
      break;
    case '2':
      // Cell imbalance
      warning[2] = warning[2] | BIT_FLAG_4;
      break;
    case '3':
      // Battery high current charge
      warning[3] = warning[3] | BIT_FLAG_1;
      break;
    case '4':
      // Battery high current charge
      warning[3] = warning[3] | BIT_FLAG_2;
      break;
    case '5':
      // Battery high current charge
      warning[3] = warning[3] | BIT_FLAG_3;
      break;
    case '6':
      // Battery high current charge
      warning[3] = warning[3] | BIT_FLAG_4;
      break;
    case '7':
      // Low SOH
      // SOH shown in menu 320.01#Soh
      // Does this trigger a warning? No
      stateOfHealth = 3;  // 3%
      break;
    case '8':
      // 0ms message interval
      // Does this impact the delivery of faults? No
      messageInterval = 0;
      measurementCycleDelay = 2000;
      break;
  }

  if (command != '.') {
    currentCommand = command;
  
    // Reset the timer
    startTime = millis();

    printBmsValues();
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      // Resubscribe
      client.subscribe(topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println("Sketch: can_bus_fake_data");
  
  client.setServer(server, 1883);
  client.setCallback(callback);

  Ethernet.begin(mac, ip);
    
  // For breakout boards with an 8 MHz clock source (default is 16 Mhz)
  const byte clock = MCP_8MHz;
  
  while (CAN_OK != CAN.begin(CAN_500KBPS, clock)) {
    Serial.println("ERROR Unable to start the CAN bus");
    delay(100);
  }

  // Reset values
  resetBmsValues();
  startTime = millis();

  // Allow the hardware to sort itself out
  delay(1500);
  
  Serial.println("Init OK");

  Serial.print("MQTT topic: ");
  Serial.println(topic);
  printBmsValues();
}

void loop() {
  // Process MQTT messages
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Send CAN bus data to the Sunny Island
  //Serial.print("Sending CAN bus messages... ");
  byte message[8] = {0, 0, 0, 0, 0, 0, 0, 0};

  if (sendMessage351) {
    message[0] = lowByte(finalChargeVoltage);
    message[1] = highByte(finalChargeVoltage);
    message[2] = lowByte(maxChargeCurrent);
    message[3] = highByte(maxChargeCurrent);
    message[4] = lowByte(maxDischargeCurrent);
    message[5] = highByte(maxDischargeCurrent);
    message[6] = lowByte(finalDischargeVoltage);
    message[7] = highByte(finalDischargeVoltage);
    CAN.sendMsgBuf(0x351, 0, 8, message);
    delay(messageInterval);
  }

  if (sendMessage355) {
    message[0] = lowByte(stateOfCharge);
    message[1] = highByte(stateOfCharge);
    message[2] = lowByte(stateOfHealth);
    message[3] = highByte(stateOfHealth);
    message[4] = lowByte(stateOfChargeHighPrecision);
    message[5] = highByte(stateOfChargeHighPrecision);
    message[6] = 0;
    message[7] = 0;
    CAN.sendMsgBuf(0x355, 0, 6, message);
    delay(messageInterval);
  }

  if (sendMessage356) {
    message[0] = lowByte(batteryVoltage);
    message[1] = highByte(batteryVoltage);
    message[2] = lowByte(batteryCurrent);
    message[3] = highByte(batteryCurrent);
    message[4] = lowByte(batteryTemp);
    message[5] = highByte(batteryTemp);
    message[6] = 0;
    message[7] = 0;
    CAN.sendMsgBuf(0x356, 0, 6, message);
    delay(messageInterval);
  }
  
  if (sendMessage35A) {
    message[0] = fault[0];
    message[1] = fault[1];
    message[2] = fault[2];
    message[3] = fault[3];
    message[4] = warning[0];
    message[5] = warning[1];
    message[6] = warning[2];
    message[7] = warning[3];
    CAN.sendMsgBuf(0x35A, 0, 8, message);
    delay(messageInterval);
  }

  if (sendMessage35E) {
    // BMS name
    message[0] = 0;
    message[1] = 0;
    message[2] = 0;
    message[3] = 0;
    message[4] = 0;
    message[5] = 0;
    message[6] = 0;
    message[7] = 0;
    bmsName.toCharArray(message, 8);
    CAN.sendMsgBuf(0x35E, 0, 8, message);
    delay(messageInterval);
  }

  if (sendMessage35F) {
    message[0] = 0x4C;  // battery chemistry low byte (char 'L')
    message[1] = 0x69;  // battery chemistry high byte (char 'i')
    message[2] = 0x04;  // BMS hardware version low byte (unsigned char)
    message[3] = 0x0F;  // BMS hardware version high byte (unsigned char)
    message[4] = lowByte(batteryCapacity);
    message[5] = highByte(batteryCapacity);
    message[6] = 0x11;  // BMS software version low byte (unsigned char)
    message[7] = 0x17;  // BMS software version high byte (unsigned char)
    CAN.sendMsgBuf(0x35F, 0, 8, message);
    delay(messageInterval);
  }
  
  //Serial.println("done");

  delay(measurementCycleDelay);
}
