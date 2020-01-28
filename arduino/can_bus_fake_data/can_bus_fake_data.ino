/**
 * Library: https://github.com/Seeed-Studio/CAN_BUS_Shield
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
#include <mcp_can.h>

const int SERIAL_BAUD = 9600;

// Create the CAN interface
MCP_CAN CAN(10);  // using CS pin 10

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println("Sketch: can_bus_fake_data");
    
  // For breakout boards with an 8 MHz clock source (default is 16 Mhz)
  const byte clock = MCP_8MHz;
  
  while (CAN_OK != CAN.begin(CAN_500KBPS, clock)) {
    Serial.println("ERROR Unable to start the CAN bus");
    delay(100);
  }
  
  Serial.println("Init OK");
}

void loop() {
  Serial.print("Sending messages... ");

  unsigned long finalChargeVoltage = 570;  // 57.0V
  signed long maxChargeCurrent = 100;  // 10.0A
  signed long maxDischargeCurrent = 120;  // 12.0A
  unsigned long finalDischargeVoltage = 490;  // 49.0V
  
  unsigned long stateOfCharge = 52;  // 52%
  unsigned long stateOfHealth = 100;  // 100%
  unsigned long stateOfChargeHighPrecision = 5230;  // 52.30%

  signed long batteryVoltage = 5320;  // 53.20V
  signed long batteryCurrent = 24;  // 2.4A
  signed long batteryTemp = 241;  // 24.1C

  signed long batteryCapacity = 1250;  // 1250Ah

  String bmsName = "TinyBMS";

  byte message[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  message[0] = lowByte(finalChargeVoltage);
  message[1] = highByte(finalChargeVoltage);
  message[2] = lowByte(maxChargeCurrent);
  message[3] = highByte(maxChargeCurrent);
  message[4] = lowByte(maxDischargeCurrent);
  message[5] = highByte(maxDischargeCurrent);
  message[6] = lowByte(finalDischargeVoltage);
  message[7] = highByte(finalDischargeVoltage);
  CAN.sendMsgBuf(0x351, 0, 8, message);
  delay(210);

  message[0] = lowByte(stateOfCharge);
  message[1] = highByte(stateOfCharge);
  message[2] = lowByte(stateOfHealth);
  message[3] = highByte(stateOfHealth);
  message[4] = lowByte(stateOfChargeHighPrecision);
  message[5] = highByte(stateOfChargeHighPrecision);
  message[6] = 0;
  message[7] = 0;
  CAN.sendMsgBuf(0x355, 0, 6, message);
  delay(210);

  message[0] = lowByte(batteryVoltage);
  message[1] = highByte(batteryVoltage);
  message[2] = lowByte(batteryCurrent);
  message[3] = highByte(batteryCurrent);
  message[4] = lowByte(batteryTemp);
  message[5] = highByte(batteryTemp);
  message[6] = 0;
  message[7] = 0;
  CAN.sendMsgBuf(0x356, 0, 6, message);
  delay(210);

  message[0] = 0;
  message[1] = 0;
  message[2] = 0;
  message[3] = 0;
  message[4] = 0;
  message[5] = 0;
  message[6] = 0;
  message[7] = 0;
  CAN.sendMsgBuf(0x35A, 0, 8, message);
  delay(210);

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
  delay(210);

  message[0] = 0x4C;  // battery chemistry low byte (unsigned integer)
  message[1] = 0x69;  // battery chemistry high byte (unsigned integer)
  message[2] = 0x04;  // BMS hardware version low byte (unsigned char)
  message[3] = 0x0F;  // BMS hardware version high byte (unsigned char)
  message[4] = lowByte(batteryCapacity);
  message[5] = highByte(batteryCapacity);
  message[6] = 0x11;  // BMS software version low byte (unsigned char)
  message[7] = 0x17;  // BMS software version high byte (unsigned char)
  CAN.sendMsgBuf(0x35F, 0, 8, message);
  delay(210);
  
  Serial.println("done");

  delay(740);
}
