#include "hardware.h"
#include "modbusdevice.h"

int main(void)
{
	ModbusDevice::openSerial("/dev/ttyS0", 115200, 'N', 8, 1);
	InitHardware();
	return 0;
}
