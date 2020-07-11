#include "util.h"
#include "esp8266.h"

char wifi_buf[256];

int send_cmd(char *cmd, char *resp) {

	int bytes = 0;

	wifiSerial->write(cmd);
	wifiSerial->flush();
	delay(100);

	if (wifiSerial->available() > 0) {
		bytes = wifiSerial->readBytes(resp, wifiSerial->available());
		serial_bprintf(buf, "%s\r\n", resp);
	}

	return bytes;

}

int wifi_send_data(uint8_t *data, int len) {

	char cmd[20];

	// Request data send with length
	sprintf(cmd, "AT+CIPSEND=%u\r\n", len);
	//send_cmd(cmd, wifi_buf);
	wifiSerial->write(cmd);
	wifiSerial->flush();
	delay(100);

	// flush input
	while (wifiSerial->available() > 0) {
		wifiSerial->read();
	}

	wifiSerial->write(data, len);
	wifiSerial->flush();
	delay(100);

	// flush input
	while (wifiSerial->available() > 0) {
		serial_bprintf(buf, "%c", wifiSerial->read());
	}

	delay(200);

}

int init_wifi() {

	int result = 1;

	DEBUGP("init_wifi\r\n");

	wifiSerial->begin(WIFI_BAUD);

	// flush input
	while (wifiSerial->available() > 0) {
		wifiSerial->read();
	}

	// Reset module
//	send_cmd("AT+RST\r\n", wifi_buf);

	// probe for wifi being present
	if (send_cmd("AT\r\n", wifi_buf) > 2) {
		if (strncmp(wifi_buf, "AT\r\r\n\r\nOK", 128) >= 0) {
			serial_bprintf(buf, "wifi response: %s\r\n", wifi_buf);
		} else {
			return 0;
		}
	} else {
		return 0;
	}

	// Check version
	//send_cmd("AT+GMR\r\n", wifi_buf);


	// Set mode to access point
	send_cmd("AT+CWMODE=2\r\n", wifi_buf);

	// Set access point params: <ssid>,<pwd>,<chl>, <ecn>
	// ecn : 0 OPEN, 2 WPA_PSK, 3 WPA2_PSK, 4 WPA_WPA2_PSK
	send_cmd("AT+CWSAP=\"tinybms\",\"\",6,0\r\n", wifi_buf);

	// Set IP
	send_cmd("AT+CIPAP=\"10.10.10.10\",\"10.10.10.1\",\"255.255.255.0\"\r\n", wifi_buf);

	// Check config
	//send_cmd("AT+CIFSR\r\n", wifi_buf);

	// setup UDP target
	send_cmd("AT+CIPSTART=\"UDP\",\"255.255.255.255\",1080\r\n", wifi_buf);

	return result;

}
