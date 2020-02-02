#include <stdio.h>
#include <stdlib.h>
#include "canbus_util.h"

int main(int argc, char** argv) {

    unsigned long finalChargeVoltage = 570; // 57.0V
    signed long maxChargeCurrent = 100; // 10.0A
    signed long maxDischargeCurrent = 120; // 12.0A
    unsigned long finalDischargeVoltage = 490; // 49.0V

    unsigned long stateOfCharge = 52; // 52%
    unsigned long stateOfHealth = 100; // 100%
    unsigned long stateOfChargeHighPrecision = 5230; // 52.30%

    signed long batteryVoltage = 5320; // 53.20V
    signed long batteryCurrent = 24; // 2.4A
    signed long batteryTemp = 241; // 24.1C

    signed long batteryCapacity = 1250; // 1250Ah    
    
    char *frame = make_charge_params_frame(finalChargeVoltage, maxChargeCurrent, maxDischargeCurrent, finalDischargeVoltage);
    
    long a, b, c, d;
    
    parse_charge_params_frame(frame, &a, &b, &c, &d);
    
    free(frame);
    
    printf("finalChargeVoltage = %lu\n", a);
    printf("maxChargeCurrent = %lu\n", b);
    printf("maxDischargeCurrent = %lu\n", c);
    printf("finalDischargeVoltage = %lu\n", d);
    
    return (EXIT_SUCCESS);
}
