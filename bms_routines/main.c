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
    
    byte *frame = make_charge_params_frame(finalChargeVoltage, maxChargeCurrent, maxDischargeCurrent, finalDischargeVoltage);
   
    word a, b, c, d;
    
    parse_charge_params_frame(frame, &a, &b, &c, &d);
    
    free(frame);
    
    printf("finalChargeVoltage = %hu\n", a);
    printf("maxChargeCurrent = %hu\n", b);
    printf("maxDischargeCurrent = %hu\n", c);
    printf("finalDischargeVoltage = %hu\n", d);
    
    return (EXIT_SUCCESS);
}
