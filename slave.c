#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "logger.h"

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

	modbus_mapping_t *modbus_mapping = modbus_mapping_new(2, 0, 2, 0);
	if (modbus_mapping == NULL) {
		LOG_ERR("Failed initialization of mapping: %s", modbus_strerror(errno));
		modbus_free(modbus_ctx);
		return -1;
	}

	if (modbus_set_slave(modbus_ctx, slave_id) == -1) {
		LOG_ERR("Failed setting slave: %s", modbus_strerror(errno));
		modbus_mapping_free(modbus_mapping);
		modbus_free(modbus_ctx);
		return -1;
	}

	if (modbus_connect(modbus_ctx) == -1) {
		LOG_ERR("Failed connecting: %s", modbus_strerror(errno));
		modbus_mapping_free(modbus_mapping);
		modbus_free(modbus_ctx);
		return -1;
	}

	modbus_mapping->tab_registers[0] = 0x0001;
	modbus_mapping->tab_registers[1] = 0x0002;

	u8 *req = malloc(MODBUS_RTU_MAX_ADU_LENGTH);
	i16 len = 0;
	do {
		do {
			len = modbus_receive(modbus_ctx, req);
		} while (len == 0);
		if (len == -1) {
			LOG_ERR("Failed receiving: %s", modbus_strerror(errno));
			break;
		}

		LOG_INFO("Received %d bytes", len);

		len = modbus_reply(modbus_ctx, req, len, modbus_mapping);
		if (len == -1) {
			LOG_ERR("Failed replying: %s", modbus_strerror(errno));
			break;
		}
	} while (true);

	modbus_mapping_free(modbus_mapping);
	modbus_close(modbus_ctx);
	modbus_free(modbus_ctx);
	free(req);
}