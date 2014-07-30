#include "hardware.h"
#include "modbusdevice.h"
#include "hardware.h"

#include "control.h"
#include "dlg_control.h"

#include <QtGui>

// #define DEBUG_TOOL

#ifndef DEBUG_TOOL

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	control();

	ControlDlg *mainDlg = new ControlDlg;
	mainDlg->show();

	return app.exec();

	return 0;
}

#else

#include <modbus.h>

static modbus_t *mb = 0;

static void closeSerial()
{
	if (mb) {
		modbus_close(mb);
		modbus_free(mb);
		mb = NULL;
	}
}

static int openSerial(const char *device, int baudrate, char parity, int data_bits, int stop_bits)
{
	int ret = 0;
	if (!mb) {
		mb = modbus_new_rtu(device, baudrate, parity, data_bits, stop_bits);
		struct timeval tv = { 0, 100000 };
		modbus_set_response_timeout(mb, &tv);
		modbus_set_debug(mb, 1);
		ret = modbus_connect(mb);
		if (ret < 0)
			closeSerial();
	}
	return ret;
}

static int mbReadReg(int address, int idx, int count, uint16_t *values)
{
	modbus_set_slave(mb, address);
	return modbus_read_registers(mb, idx, count, values);
}

int main(int argc, char *argv[])
{
	int ret;

	if (argc != 4) {
		printf("Usage: %s addr idx count\n", argv[0]);
		return 1;
	}

	int addr = atoi(argv[1]);
	int idx = atoi(argv[2]);
	int count = atoi(argv[3]);
	uint16_t values[256];

	ret = openSerial("/dev/ttyUSB0", 38400, 'N', 8, 1);
	if (ret) {
		perror("Cannot open serial");
		return ret;
	}

	ret = mbReadReg(addr, idx, count, values);
	if (ret != count) {
		perror("Cannot read register");
		return ret;
	}

	printf("Reading %i regs at %i from %i (%02x):\n", count, idx, addr, addr);
	for (int i=0; i<count; ++i)
		printf("%02x[%3i]=%04x (%i)\n", addr, idx+i, values[i], values[i]);

	closeSerial();
	return 0;
}

#endif
