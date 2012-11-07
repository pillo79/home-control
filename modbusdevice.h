#ifndef __MODBUSDEVICE_H__
#define __MODBUSDEVICE_H__

#include <stdio.h>
#include <stdint.h>

class ModbusDevice {
		int mModAddr;
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
		virtual int getInputReg(int input) { return -ENOTSUP; }
		virtual int getOutputReg(int output) { return -ENOTSUP; }
		virtual int setOutputReg(int output, int value) { return -ENOTSUP; }
	protected:
		int mbRead4x(int idx, int count, short *values);
		int mbWrite4x(int idx, int count, short *values);
};

class Seneca10DIN : public ModbusDevice {
	public:
		Seneca_10DI(int modAddress);
		virtual ~Seneca_10DI() { };

		virtual void updateInputs();

		virtual void getDigInput(int input);
	private:
		uint16_t mInputs;
};

class Seneca_16DI_8DO : public ModbusDevice {
	public:
		Seneca_16DI_8DO(int modAddress);
		virtual ~Seneca_16DI_8DO() { };

		virtual void updateInputs();
		virtual void updateOutputs();

		virtual void getDigInput(int input);
		virtual int getDigOutput(int output);
		virtual int setDigOutput(int output, bool value);
	private:
		uint16_t mInputs;
		uint16_t mOutputs;
};

#endif /* __MODBUSDEVICE_H__ */
