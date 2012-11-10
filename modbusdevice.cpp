#include <modbus.h>
#include "modbusdevice.h"

static modbus_t *mb = 0;

int ModbusDevice::openSerial(const char *device, int baudrate, char parity, int data_bits, int stop_bits)
{
	int ret = 0;
	if (!mb) {
		mb = modbus_new_rtu(device, baudrate, parity, data_bits, stop_bits);
		ret = modbus_connect(mb);
		if (ret < 0)
			closeSerial();
	}
	return ret;
}

void ModbusDevice::closeSerial()
{
	if (mb) {
		modbus_close(mb);
		modbus_free(mb);
		mb = NULL;
	}
}

ModbusDevice::ModbusDevice(int modAddress)
	: mAddress (modAddress)
{
}

int ModbusDevice::mbRead4x(int idx, int count, uint16_t *values)
{
	modbus_set_slave(mb, mAddress);
	return modbus_read_registers(mb, idx, count, values);
}

int ModbusDevice::mbWrite4x(int idx, int count, const uint16_t *values)
{
	modbus_set_slave(mb, mAddress);
	return modbus_write_registers(mb, idx, count, values);
}



/* Z-10-D-IN */
Seneca_10DI::Seneca_10DI(int modAddress)
	: ModbusDevice(modAddress)
	, mInputs(0)
{
}

void Seneca_10DI::updateInputs()
{
	mbRead4x(40002, 1, &mInputs);
}

int Seneca_10DI::getDigInput(int input)
{
	if (input < 1) return -ENOTSUP;
	if (input > 10) return -ENOTSUP;
	return mInputs & (1 << (input-1));
}



/* Z-10-D-OUT */
Seneca_10DO::Seneca_10DO(int modAddress)
	: ModbusDevice(modAddress)
	, mOutputs(0)
{
}

void Seneca_10DO::updateOutputs()
{
	mbWrite4x(40003, 1, &mOutputs);
}

int Seneca_10DO::getDigOutput(int output)
{
	if (output < 1) return -ENOTSUP;
	if (output > 10) return -ENOTSUP;
	return mOutputs & (1 << (output-1));
}

int Seneca_10DO::setDigOutput(int output, bool value)
{
	if (output < 1) return -ENOTSUP;
	if (output > 10) return -ENOTSUP;
	if (value)
		mOutputs |= (1 << (output-1));
	else
		mOutputs &= ~(1 << (output-1));

	return 0;
}



/* ZC-16DI-8DO */
Seneca_16DI_8DO::Seneca_16DI_8DO(int modAddress)
	: ModbusDevice(modAddress)
	, mInputs(0)
	, mOutputs(0)
{
}

void Seneca_16DI_8DO::updateInputs()
{
	mbRead4x(40301, 1, &mInputs);
}

void Seneca_16DI_8DO::updateOutputs()
{
	mbWrite4x(40005, 1, &mInputs);
}

int Seneca_16DI_8DO::getDigInput(int input)
{
	if (input < 1) return -ENOTSUP;
	if (input > 16) return -ENOTSUP;
	return mInputs & (1 << (input-1));
}

int Seneca_16DI_8DO::getDigOutput(int output)
{
	if (output < 1) return -ENOTSUP;
	if (output > 8) return -ENOTSUP;
	return mOutputs & (1 << (output-1));
}

int Seneca_16DI_8DO::setDigOutput(int output, bool value)
{
	if (output < 1) return -ENOTSUP;
	if (output > 8) return -ENOTSUP;
	if (value)
		mOutputs |= (1 << (output-1));
	else
		mOutputs &= ~(1 << (output-1));
}



/* Z-4RTD-2 */
Seneca_4RTD::Seneca_4RTD(int modAddress)
	: ModbusDevice(modAddress)
{
	for (int i=0; i<4; ++i)
		mInputs[i] = 0;
}

void Seneca_4RTD::updateInputs()
{
	mbRead4x(40003, 4, mInputs);
}

int Seneca_4RTD::getInputVal(int input)
{
	if (input < 1) return -ENOTSUP;
	if (input > 4) return -ENOTSUP;
	return mInputs[input-1];
}



/* Z-4AI */
Seneca_4AI::Seneca_4AI(int modAddress)
	: ModbusDevice(modAddress)
{
	for (int i=0; i<4; ++i)
		mInputs[i] = 0;
}

void Seneca_4AI::updateInputs()
{
	mbRead4x(40017, 4, mInputs);
}

int Seneca_4AI::getInputVal(int input)
{
	if (input < 1) return -ENOTSUP;
	if (input > 4) return -ENOTSUP;
	return mInputs[input-1];
}



/* Z-3AO */
Seneca_3AO::Seneca_3AO(int modAddress)
	: ModbusDevice(modAddress)
{
	for (int i=0; i<3; ++i)
		mOutputs[i] = 0;
}

void Seneca_3AO::updateOutputs()
{
	mbWrite4x(40005, 3, mOutputs);
}

int Seneca_3AO::getOutputVal(int output)
{
	if (output < 1) return -ENOTSUP;
	if (output > 3) return -ENOTSUP;
	return mOutputs[output-1];
}

int Seneca_3AO::setOutputVal(int output, int val)
{
	if (output < 1) return -ENOTSUP;
	if (output > 3) return -ENOTSUP;

	mOutputs[output-1] = val;
	return 0;
}


