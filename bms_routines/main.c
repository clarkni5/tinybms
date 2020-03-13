#include <stdio.h>
#include <stdlib.h>
#include "canbus_util.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <assert.h>
#include <time.h>

int main(int argc, char** argv) {

    struct sockaddr_in si_me, si_other;
    int s;
    assert((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) != -1);
    int port = 1080;
    int broadcast = 1;

    setsockopt(s, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof (broadcast));

    memset(&si_me, 0, sizeof (si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(port);
    si_me.sin_addr.s_addr = INADDR_ANY;

    assert(bind(s, (struct sockaddr *) & si_me, sizeof (struct sockaddr)) != -1);

    while (1) {
	char buf[2048];
	unsigned slen = sizeof (struct sockaddr);
	int bytes = recvfrom(s, buf, sizeof (buf) - 1, 0, (struct sockaddr *) & si_other, &slen);

	uint32_t id = *((uint32_t*) buf);
	time_t t = time(NULL);

	switch (id) {

	    case SI_CHARGE_PARAMS_FRAME:
	    {
		uint16_t finalChargeVoltage, maxChargeCurrent, maxDischargeCurrent, finalDischargeVoltage;
		parse_charge_params_frame(&buf[4], &finalChargeVoltage, &maxChargeCurrent, &maxDischargeCurrent, &finalDischargeVoltage);
		printf("%s Charge params: finalVoltage %hu, maxChargeCurrent %hu, maxDischargeCurrent %hu, finalDischargeVoltage %hu\n", ctime(&t), finalChargeVoltage,
			maxChargeCurrent, maxDischargeCurrent, finalDischargeVoltage);
		break;
	    }
	    case SI_VOLTAGE_FRAME:
	    {
		uint16_t batteryVoltage, batteryCurrent, batteryTemp;
		parse_voltage_frame(&buf[4], &batteryVoltage, &batteryCurrent, &batteryTemp);
		printf("%s Voltage params: batteryVoltage %hu, batteryCurrent %hu, batteryTemp %hu\n", ctime(&t), batteryVoltage, batteryCurrent, batteryTemp);
		break;
	    }
	    case SI_SOC_FRAME:
	    {
		uint16_t stateOfCharge, stateOfHealth, stateOfChargeHighPrecision;
		parse_soc_frame(&buf[4], &stateOfCharge, &stateOfHealth, &stateOfChargeHighPrecision);
		printf("%s States: stateOfCharge %hu, stateOfHealth %hu, stateofChargeHighPrecision %hu\n", ctime(&t), stateOfCharge, stateOfHealth, stateOfChargeHighPrecision);
		break;
	    }
	    case SI_ID_FRAME:
	    {

		struct _id_frame {
		    uint16_t a;
		    uint16_t b;
		    uint16_t capacity;
		    uint16_t c;

		} id_frame;

		memcpy(&id_frame, &buf[4], sizeof (id_frame));
		printf("%s ID: capacity %hu\n", ctime(&t), id_frame.capacity);

		break;
	    }
	    case SI_NAME_FRAME:
	    {

		char bms_name[100];
		memcpy(bms_name, &buf[4], bytes - 4);
		bms_name[bytes - 4] = 0;
		printf("%s BMS Name: %s\n", ctime(&t), bms_name);

		break;
	    }
	    case SI_FAULT_FRAME:
	    {

		struct _faults {
		    uint8_t f0, f1, f2, f3;
		} faults;

		memcpy(&faults, &buf[4], sizeof (faults));
		printf("%s Faults: f0 %hhx, f1 %hhx, f2 %hhx, f3 %hhx\n", ctime(&t), faults.f0, faults.f1, faults.f2, faults.f3);

		break;
	    }

	    default:
		printf("unknown frame type %x\n", id);
		break;

	}

    }

    return (EXIT_SUCCESS);

}
