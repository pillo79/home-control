#include <modbus.h>
#include "modbusdevice.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <termios.h>

static modbus_t *mb = 0;
static int busBaudrate;

int ModbusDevice::openSerial(const char *device)
{
	int ret = 0;
	if (!mb) {
		busBaudrate = B9600;
		mb = modbus_new_rtu(device, 9600, 'N', 8, 1);
		struct timeval tv = { 0, 100000 };
		modbus_set_response_timeout(mb, &tv);
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

ModbusDevice::ModbusDevice(int modAddress, int modBaudrate)
	: mAddress (modAddress)
	, mBaudrate (modBaudrate)
	, mFailures (0)
{
}

void ModbusDevice::mbSetBaudrate(int baudrate)
{
	if (baudrate != busBaudrate) {
    		struct termios tios;

		int s = modbus_get_socket(mb);
		tcgetattr(s, &tios);
		cfsetispeed(&tios, baudrate);
		cfsetospeed(&tios, baudrate);
		tcsetattr(s, TCSANOW, &tios);

		busBaudrate = baudrate;
	}
}

int ModbusDevice::mbReadReg(int idx, int count, uint16_t *values)
{
	int ret = -ENOTSUP;
	mbSetBaudrate(mBaudrate);
	modbus_set_slave(mb, mAddress);

	if ((idx > 30000) && (idx < 40000)) {
		ret = modbus_read_input_registers(mb, idx-30001, count, values);
		if (ret < 0) {
			if (mFailures < 60) mFailures += 5;
			fprintf(stderr, "R err %s addr %i\n", strerror(errno), mAddress);
		} else
			if (mFailures) --mFailures;
	} else if ((idx > 40000) && (idx < 50000)) {
		ret = modbus_read_registers(mb, idx-40001, count, values);
		if (ret < 0) {
			if (mFailures < 60) mFailures += 5;
			fprintf(stderr, "R err %s addr %i\n", strerror(errno), mAddress);
		} else
			if (mFailures) --mFailures;
	}

	return ret;
}

int ModbusDevice::mbWriteReg(int idx, int count, const uint16_t *values)
{
	int ret = -ENOTSUP;
	mbSetBaudrate(mBaudrate);
	modbus_set_slave(mb, mAddress);

	if ((idx > 40000) && (idx < 50000)) {
		int ret = modbus_write_registers(mb, idx-40001, count, values);
		if (ret < 0) {
			if (mFailures < 60) mFailures += 5;
			fprintf(stderr, "W err %s addr %i\n", strerror(errno), mAddress);
		} else
			if (mFailures) --mFailures;
	}

	return ret;
}



/* Z-10-D-IN */
Seneca_10DI::Seneca_10DI(int modAddress)
	: ModbusDevice(modAddress, B9600)
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
	: ModbusDevice(modAddress, B9600)
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



/* ZC-24DO */
Seneca_24DO::Seneca_24DO(int modAddress)
	: ModbusDevice(modAddress, B9600)
	, mOutputs(0)
{
}

int Seneca_24DO::updateOutputs()
{
	uint16_t words[2] = {
		mOutputs & 0xffff,
		mOutputs >> 16
	};

	return mbWriteReg(40301, 2, words);
}

int Seneca_24DO::getDigOutput(int output)
{
	if (output < 1) return -ENOTSUP;
	if (output > 24) return -ENOTSUP;
	return mOutputs & (1 << (output-1));
}

int Seneca_24DO::setDigOutput(int output, bool value)
{
	if (output < 1) return -ENOTSUP;
	if (output > 24) return -ENOTSUP;
	if (value)
		mOutputs |= (1 << (output-1));
	else
		mOutputs &= ~(1 << (output-1));

	return 0;
}



/* ZC-16DI-8DO */
Seneca_16DI_8DO::Seneca_16DI_8DO(int modAddress)
	: ModbusDevice(modAddress, B9600)
	, mInputs(0)
	, mOutputs(0)
{
}

int Seneca_16DI_8DO::updateInputs()
{
	int ret;

	// read digital inputs
	ret = mbReadReg(40301, 1, &mInputs);
	if (ret < 1) return ret;
#if 0	// disabled to optimize speed
	// read counters
	ret = mbReadReg(40009, 16, mCounters);
#endif
	return ret;
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

int Seneca_16DI_8DO::getInputVal(int input)
{
#if 0	// disabled to optimize speed
	if (input < 1) return -ENOTSUP;
	if (input > 8) return -ENOTSUP;

	input = (input-1) << 1;
	return (mCounters[input] << 16) | mCounters[input+1];
#else
	return -ENOTSUP;
#endif
}



/* Z-4RTD-2 */
Seneca_4RTD::Seneca_4RTD(int modAddress)
	: ModbusDevice(modAddress, B9600)
{
	for (int i=0; i<4; ++i)
		mInputs[i] = 0;
}

int Seneca_4RTD::updateInputs()
{
	int ret = mbReadReg(40003, 4, (uint16_t*) mInputs);
	if (ret < 0) {
		uint16_t reset_code = 0xcccc;
		mbWriteReg(40029, 1, &reset_code);
	}
	return ret;
}

int Seneca_4RTD::getInputVal(int input)
{
	if (input < 1) return -ENOTSUP;
	if (input > 4) return -ENOTSUP;
	return mInputs[input-1];
}



/* Z-4AI */
Seneca_4AI::Seneca_4AI(int modAddress)
	: ModbusDevice(modAddress, B9600)
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
	: ModbusDevice(modAddress, B9600)
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



/* Sensore potenza */
Eastron_SDM230::Eastron_SDM230(int modAddress)
	: ModbusDevice(modAddress, B9600)
{
	for (int i=0; i<4; ++i)
		mInputs[i] = 0;
}

int Eastron_SDM230::updateInputs()
{
	int ret = mbReadReg(30013, 2, mInputs);
	if (ret < 2) return ret;
	return mbReadReg(30343, 2, mInputs+2);
}

int Eastron_SDM230::getInputVal(int input)
{
	switch (input) {
		case 30013:	return mInputs[0];
		case 30014:	return mInputs[1];
		case 30343:	return mInputs[2];
		case 30344:	return mInputs[3];
		default:	return -ENOTSUP;
	}
}



/* Sensore potenza con CT */
Eastron_SDM120CT::Eastron_SDM120CT(int modAddress)
	: ModbusDevice(modAddress, B9600)
{
	for (int i=0; i<4; ++i)
		mInputs[i] = 0;
}

int Eastron_SDM120CT::updateInputs()
{
	int ret = mbReadReg(30013, 2, mInputs);
	if (ret < 2) return ret;
	return mbReadReg(30343, 2, mInputs+2);
}

int Eastron_SDM120CT::getInputVal(int input)
{
	switch (input) {
		case 30013:	return mInputs[0];
		case 30014:	return mInputs[1];
		case 30343:	return mInputs[2];
		case 30344:	return mInputs[3];
		default:	return -ENOTSUP;
	}
}



/* Sensore temp/umidita */
Burosoft_Temp::Burosoft_Temp(int modAddress)
	: ModbusDevice(modAddress, B9600)
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
