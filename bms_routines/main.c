#include <stdio.h>
#include <stdlib.h>
#include "canbus_util.h"

int main(int argc, char** argv) {

    word finalChargeVoltage = 570; // 57.0V
    word maxChargeCurrent = 100; // 10.0A
    word maxDischargeCurrent = 120; // 12.0A
    word finalDischargeVoltage = 490; // 49.0V
    word stateOfCharge = 52; // 52%
    word stateOfHealth = 100; // 100%
    word stateOfChargeHighPrecision = 5230; // 52.30%
    word batteryVoltage = 5320; // 53.20V
    word batteryCurrent = 24; // 2.4A
    word batteryTemp = 241; // 24.1C
    word batteryCapacity = 1250; // 1250Ah    
    
    printf("sizeof(byte) = %lu\n", sizeof(byte));
    printf("sizeof(word) = %lu\n\n", sizeof(word));
    
    byte *frame = make_charge_params_frame2(finalChargeVoltage, maxChargeCurrent, maxDischargeCurrent, finalDischargeVoltage);
   
    word a, b, c, d;
    
    parse_charge_params_frame(frame, &a, &b, &c, &d);
    
    free(frame);
    
    printf("finalChargeVoltage = %hu\n", a);
    printf("maxChargeCurrent = %hu\n", b);
    printf("maxDischargeCurrent = %hu\n", c);
    printf("finalDischargeVoltage = %hu\n", d);
    
    frame = make_soc_frame(stateOfCharge, stateOfHealth, stateOfChargeHighPrecision);
    
    parse_soc_frame(frame, &a, &b, &c);
    
    printf("stateOfCharge = %hu\n", a);
    printf("stateOfHealth = %hu\n", b);
    printf("stateOfChargeHighPrecision = %hu\n", c);
    
    free(frame);
    
    frame = make_voltage_frame(batteryVoltage, batteryCurrent, batteryTemp);
    
    parse_voltage_frame(frame, &a, &b, &c);
    
    printf("batteryVoltage = %hu\n", a);
    printf("batteryCurrent = %hu\n", b);
    printf("batteryTemp = %hu\n", c);
    
    free(frame);
    
    return (EXIT_SUCCESS);
}
