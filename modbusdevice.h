#ifndef __MODBUSDEVICE_H__
#define __MODBUSDEVICE_H__

#include <errno.h>
#include <stdio.h>
#include <stdint.h>

class ModbusDevice {
		int mAddress;
	public:
		static int openSerial(const char *device, int baudrate, char parity, int data_bits, int stop_bits);
		static void closeSerial();
	public:
		ModbusDevice(int modAddress);
		virtual ~ModbusDevice() { };

		virtual void updateInputs() { };
		virtual void updateOutputs() { };

		virtual int getDigInput(int input) { return -ENOTSUP; }
		virtual int getDigOutput(int output) { return -ENOTSUP; }
		virtual int setDigOutput(int output, bool value) { return -ENOTSUP; }
		virtual int getInputVal(int input) { return -ENOTSUP; }
		virtual int getOutputVal(int output) { return -ENOTSUP; }
		virtual int setOutputVal(int output, int value) { return -ENOTSUP; }
	protected:
		int mbRead4x(int idx, int count, uint16_t *values);
		int mbWrite4x(int idx, int count, const uint16_t *values);
};

class Seneca_10DI : public ModbusDevice {
	public:
		Seneca_10DI(int modAddress);
		virtual ~Seneca_10DI() { };

		virtual void updateInputs();

		virtual int getDigInput(int input);
	private:
		uint16_t mInputs;
};

class Seneca_10DO : public ModbusDevice {
	public:
		Seneca_10DO(int modAddress);
		virtual ~Seneca_10DO() { };

		virtual void updateOutputs();

		virtual int getDigOutput(int output);
		virtual int setDigOutput(int output, bool value);
	private:
		uint16_t mOutputs;
};

class Seneca_16DI_8DO : public ModbusDevice {
	public:
		Seneca_16DI_8DO(int modAddress);
		virtual ~Seneca_16DI_8DO() { };

		virtual void updateInputs();
		virtual void updateOutputs();

		virtual int getDigInput(int input);
		virtual int getDigOutput(int output);
		virtual int setDigOutput(int output, bool value);
	private:
		uint16_t mInputs;
		uint16_t mOutputs;
};

class Seneca_4RTD : public ModbusDevice {
	public:
		Seneca_4RTD(int modAddress);
		virtual ~Seneca_4RTD() { };

		virtual void updateInputs();

		virtual int getInputVal(int input);
	private:
		uint16_t mInputs[4];
};

class Seneca_4AI : public ModbusDevice {
	public:
		Seneca_4AI(int modAddress);
		virtual ~Seneca_4AI() { };

		virtual void updateInputs();

		virtual int getInputVal(int input);
	private:
		uint16_t mInputs[4];
};

class Seneca_3AO : public ModbusDevice {
	public:
		Seneca_3AO(int modAddress);
		virtual ~Seneca_3AO() { };

		virtual void updateOutputs();

		virtual int getOutputVal(int output);
		virtual int setOutputVal(int output, int value);
	private:
		uint16_t mOutputs[3];
};

#endif /* __MODBUSDEVICE_H__ */
