#include <modbus.h>
#include "modbusdevice.h"

static modbus_t *mb = 0;

int ModbusDevice::openSerial(const char *device, int baudrate, char parity, int data_bits, int stop_bits)
{
	int ret = 0;
	if (!mb) {
		mb = modbus_new_rtu(device, baudrate, parity, data_bits, stop_bits);
		struct timeval tv = { 0, 100000 };
		modbus_set_response_timeout(mb, &tv);
//		modbus_set_debug(mb, 1);
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

int ModbusDevice::mbReadReg(int idx, int count, uint16_t *values)
{
	modbus_set_slave(mb, mAddress);
	if ((idx > 40000) && (idx < 50000))
		return modbus_read_registers(mb, idx-40001, count, values);
	else
		return -ENOTSUP;
}

int ModbusDevice::mbWriteReg(int idx, int count, const uint16_t *values)
{
	modbus_set_slave(mb, mAddress);
	if ((idx > 40000) && (idx < 50000))
		return modbus_write_registers(mb, idx-40001, count, values);
	else
		return -ENOTSUP;
}



/* Z-10-D-IN */
Seneca_10DI::Seneca_10DI(int modAddress)
	: ModbusDevice(modAddress)
	, mInputs(0)
{
}

int Seneca_10DI::updateInputs()
{
	return mbReadReg(40002, 1, &mInputs);
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

int Seneca_10DO::updateOutputs()
{
	return mbWriteReg(40003, 1, &mOutputs);
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

int Seneca_16DI_8DO::updateInputs()
{
	return mbReadReg(40301, 1, &mInputs);
}

int Seneca_16DI_8DO::updateOutputs()
{
	return mbWriteReg(40005, 1, &mOutputs);
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
	return 0;
}



/* Z-4RTD-2 */
Seneca_4RTD::Seneca_4RTD(int modAddress)
	: ModbusDevice(modAddress)
{
	for (int i=0; i<4; ++i)
		mInputs[i] = 0;
}

int Seneca_4RTD::updateInputs()
{
	return mbReadReg(40003, 4, mInputs);
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

int Seneca_4AI::updateInputs()
{
	return mbReadReg(40017, 4, mInputs);
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

int Seneca_3AO::updateOutputs()
{
	return mbWriteReg(40005, 3, mOutputs);
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



/* Sensore temp/umidita */
Burosoft_Temp::Burosoft_Temp(int modAddress)
	: ModbusDevice(modAddress)
{
	for (int i=0; i<2; ++i)
		mInputs[i] = 0;
}

int Burosoft_Temp::updateInputs()
{
	return mbReadReg(40001, 2, mInputs);
}

int Burosoft_Temp::getInputVal(int input)
{
	if (input < 1) return -ENOTSUP;
	if (input > 2) return -ENOTSUP;
	return mInputs[input-1];
}
