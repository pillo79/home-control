#ifndef __MODBUSDEVICE_H__
#define __MODBUSDEVICE_H__

#include <errno.h>
#include <stdio.h>
#include <stdint.h>

#include "global.h"

class ModbusDevice {
	public:
		static int openSerial(const char *device);
		static void closeSerial();
	public:
		ModbusDevice(int modAddress, int modBaudrate);
		virtual ~ModbusDevice() { };

		virtual int updateInputs() { return 0; };
		virtual int updateOutputs() { return 0; };
		bool failing() { return (mFailures > 20); }

		virtual int getDigInput(int input) { UNUSED(input); return -ENOTSUP; }
		virtual int getDigOutput(int output) { UNUSED(output); return -ENOTSUP; }
		virtual int setDigOutput(int output, bool value) { UNUSED(output); UNUSED(value); return -ENOTSUP; }
		virtual int getInputVal(int input) { UNUSED(input); return -ENOTSUP; }
		virtual int getOutputVal(int output) { UNUSED(output); return -ENOTSUP; }
		virtual int setOutputVal(int output, int value) { UNUSED(output); UNUSED(value); return -ENOTSUP; }
	protected:
		int mAddress;
		int mBaudrate;
		int mFailures;
		int mbReadReg(int idx, int count, uint16_t *values);
		int mbWriteReg(int idx, int count, const uint16_t *values);

	private:
		static void mbSetBaudrate(int baudrate);
};

class Seneca_10DI : public ModbusDevice {
	public:
		Seneca_10DI(int modAddress);
		virtual ~Seneca_10DI() { };

		virtual int updateInputs();

		virtual int getDigInput(int input);
	private:
		uint16_t mInputs;
};

class Seneca_10DO : public ModbusDevice {
	public:
		Seneca_10DO(int modAddress);
		virtual ~Seneca_10DO() { };

		virtual int updateOutputs();

		virtual int getDigOutput(int output);
		virtual int setDigOutput(int output, bool value);
	private:
		uint16_t mOutputs;
};

class Seneca_24DO : public ModbusDevice {
	public:
		Seneca_24DO(int modAddress);
		virtual ~Seneca_24DO() { };

		virtual int updateOutputs();

		virtual int getDigOutput(int output);
		virtual int setDigOutput(int output, bool value);
	private:
		uint32_t mOutputs;
};

class Seneca_16DI_8DO : public ModbusDevice {
		public:
		Seneca_16DI_8DO(int modAddress);
		virtual ~Seneca_16DI_8DO() { };

		virtual int updateInputs();
		virtual int updateOutputs();

		virtual int getDigInput(int input);
		virtual int getDigOutput(int output);
		virtual int setDigOutput(int output, bool value);

		virtual int getInputVal(int input);
	private:
		uint16_t mInputs;
		uint16_t mOutputs;

		uint16_t mCounters[16];
};

class Seneca_4RTD : public ModbusDevice {
	public:
		Seneca_4RTD(int modAddress);
		virtual ~Seneca_4RTD() { };

		virtual int updateInputs();

		virtual int getInputVal(int input);
	private:
		int16_t mInputs[4]; // signed (negative temperatures)
};

class Seneca_4AI : public ModbusDevice {
	public:
		Seneca_4AI(int modAddress);
		virtual ~Seneca_4AI() { };

		virtual int updateInputs();

		virtual int getInputVal(int input);
	private:
		uint16_t mInputs[4];
};

class Seneca_3AO : public ModbusDevice {
	public:
		Seneca_3AO(int modAddress);
		virtual ~Seneca_3AO() { };

		virtual int updateOutputs();

		virtual int getOutputVal(int output);
		virtual int setOutputVal(int output, int value);
	private:
		uint16_t mOutputs[3];
};

class Eastron_SDM230 : public ModbusDevice {
	public:
		Eastron_SDM230(int modAddress);
		virtual ~Eastron_SDM230() { };

		virtual int updateInputs();

		virtual int getInputVal(int input);
	private:
		uint16_t mInputs[4];
};

class Burosoft_Temp : public ModbusDevice {
	public:
		Burosoft_Temp(int modAddress);
		virtual ~Burosoft_Temp() { };

		virtual int updateInputs();

		virtual int getInputVal(int input);
	private:
		uint16_t mInputs[2];
};

#endif /* __MODBUSDEVICE_H__ */
