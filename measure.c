#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "logger.h"

static u16 buffer[16];

int main(int argc, char *argv[])
{
	if (argc != 7) {
		LOG_ERR(
			"Usage: %s <serial port> <baud rate> <parity check> <data bits> <stop bits> <slave id>",
			argv[0]);
		return -1;
	}

	char serial_port[16];
	char serial_port_prefix[] = "/dev/";
	strcpy(serial_port, serial_port_prefix);
	strcat(serial_port, argv[1]);
	printf("%s\n", serial_port);

	u32 baud_rate = atoi(argv[2]);
	u32 parity_check = argv[3][0];
	u32 data_bits = atoi(argv[4]);
	u32 stop_bits = atoi(argv[5]);
	u32 slave_id = atoi(argv[6]);

	ModbusCtx *modbus_ctx =
		modbus_new_rtu(serial_port, baud_rate, parity_check, data_bits, stop_bits);
	if (modbus_ctx == NULL) {
		LOG_ERR("Failed initialization of context: %s", modbus_strerror(errno));
		return -1;
	}

	if (modbus_set_debug(modbus_ctx, true) == -1) {
		LOG_ERR("Failed setting debug: %s", modbus_strerror(errno));
		modbus_free(modbus_ctx);
		return -1;
	}

	if (modbus_set_response_timeout(modbus_ctx, 2, 0) == -1) {
		LOG_ERR("Failed setting response timeout: %s", modbus_strerror(errno));
		modbus_free(modbus_ctx);
		return -1;
	}

	if (modbus_set_slave(modbus_ctx, slave_id) == -1) {
		LOG_ERR("Failed setting slave: %s", modbus_strerror(errno));
		modbus_close(modbus_ctx);
		modbus_free(modbus_ctx);
		return -1;
	}

	if (modbus_connect(modbus_ctx) == -1) {
		LOG_ERR("Failed connecting: %s", modbus_strerror(errno));
		modbus_free(modbus_ctx);
		return -1;
	}

	if (modbus_read_registers(modbus_ctx, 0x00, 2, buffer) == -1) {
		LOG_ERR("Failed reading registers: %s", modbus_strerror(errno));
		modbus_close(modbus_ctx);
		modbus_free(modbus_ctx);
		return -1;
	}

	printf("Data 1: %d\n", buffer[0]);
	printf("Data 2: %d\n", buffer[1]);

	modbus_close(modbus_ctx);
	modbus_free(modbus_ctx);

	return 0;
}