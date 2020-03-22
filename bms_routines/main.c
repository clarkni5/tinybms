#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <assert.h>
#include <time.h>
#include "canbus_util.h"

void decode_frame(uint8_t *frame, int len) {

    uint32_t id = *((uint32_t*) frame);
    time_t t = time(NULL);
    uint8_t *data = frame + 4;

    switch (id) {

        case SI_CHARGE_PARAMS_FRAME:
        {
            uint16_t finalChargeVoltage, maxChargeCurrent, maxDischargeCurrent, finalDischargeVoltage;
            parse_charge_params_frame(data, &finalChargeVoltage, &maxChargeCurrent, &maxDischargeCurrent, &finalDischargeVoltage);
            printf("%.19s Charge params: finalVoltage %hu, maxChargeCurrent %hu, maxDischargeCurrent %hu, finalDischargeVoltage %hu\n", ctime(&t), finalChargeVoltage,
                    maxChargeCurrent, maxDischargeCurrent, finalDischargeVoltage);
            break;
        }
        case SI_VOLTAGE_FRAME:
        {
            uint16_t batteryVoltage, batteryCurrent, batteryTemp;
            parse_voltage_frame(data, &batteryVoltage, &batteryCurrent, &batteryTemp);
            printf("%.19s Voltage params: batteryVoltage %hu, batteryCurrent %hu, batteryTemp %hu\n", ctime(&t), batteryVoltage, batteryCurrent, batteryTemp);
            break;
        }
        case SI_SOC_FRAME:
        {
            uint16_t stateOfCharge, stateOfHealth, stateOfChargeHighPrecision;
            parse_soc_frame(data, &stateOfCharge, &stateOfHealth, &stateOfChargeHighPrecision);
            printf("%.19s States: stateOfCharge %hu, stateOfHealth %hu, stateofChargeHighPrecision %hu\n", ctime(&t), stateOfCharge, stateOfHealth, stateOfChargeHighPrecision);
            break;
        }
        case SI_ID_FRAME:
        {

            struct _id_frame {
                uint16_t chemistry;
                uint16_t hw_ver;
                uint16_t capacity;
                uint16_t sw_ver;

            } *id_frame = (struct _id_frame*)data;

            printf("%.19s ID: chemistry %hu, hw_ver %hu, capacity %hu, sw_ver %hu\n", ctime(&t), id_frame->chemistry, id_frame->hw_ver, id_frame->capacity, id_frame->sw_ver);

            break;
        }
        case SI_NAME_FRAME:
        {

            char bms_name[100];
            memcpy(bms_name, data, len - 4);
            bms_name[len - 4] = 0;
            printf("%.19s BMS Name: %s\n", ctime(&t), bms_name);

            break;
        }
        case SI_FAULT_FRAME:
        {

            struct _faults {
                uint8_t f0, f1, f2, f3;
            } *faults = (struct _faults*)data;

            printf("%.19s Faults: f0 %hhx, f1 %hhx, f2 %hhx, f3 %hhx\n", ctime(&t), faults->f0, faults->f1, faults->f2, faults->f3);

            break;
        }

        default:
            printf("%.19s unknown frame type %x\n", ctime(&t), id);
            break;

    }

}

int main(int argc, char** argv) {
    
    struct sockaddr_in si_me, si_other;
    int sock;
    int broadcast = 1;
    uint16_t port = 1080;
    uint8_t buf[2048], buf1[2048];

    assert((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) != -1);

    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof (broadcast));

    memset(&si_me, 0, sizeof (si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(port);
    si_me.sin_addr.s_addr = INADDR_ANY;

    assert(bind(sock, (struct sockaddr *) & si_me, sizeof (struct sockaddr)) != -1);
    
    printf("Waiting for packets...\n");

    while (1) {

        unsigned slen = sizeof (struct sockaddr);
        int bytes = recvfrom(sock, buf, sizeof (buf), 0, (struct sockaddr *) & si_other, &slen);

	// dupe?
	if(memcmp(buf, buf1, bytes) == 0)
	    continue;
	
	memcpy(buf1, buf, bytes);
        
        decode_frame(buf, slen);

    }

    return (EXIT_SUCCESS);

}
