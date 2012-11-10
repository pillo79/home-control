#ifndef __MODBUSIO_H__
#define __MODBUSIO_H__

#include <errno.h>

class ModbusDevice;

class ModbusIO {
	public:
		ModbusIO(ModbusDevice *dev);
		virtual ~ModbusIO() { }

		virtual int  getValue() { return -ENOTSUP; }
		virtual void setValue(int value) { }
	protected:
		ModbusDevice *mDev;
};

class BitInput : public ModbusIO {
		int mBitAddr;
	public:
		BitInput(ModbusDevice *dev, int bitAddr);
		virtual ~BitInput() { }

		virtual int getValue();
};

class BitOutput : public ModbusIO {
		int mBitAddr;
	public:
		BitOutput(ModbusDevice *dev, int bitAddr);
		virtual ~BitOutput() { }

		virtual int  getValue();
		virtual void setValue(int value);
};

class WordInput : public ModbusIO {
		int mWordAddr;
	public:
		WordInput(ModbusDevice *dev, int wordAddr);
		virtual ~WordInput() { }

		virtual int getValue();
};

class WordOutput : public ModbusIO {
		int mWordAddr;
	public:
		WordOutput(ModbusDevice *dev, int wordAddr);
		virtual ~WordOutput() { }

		virtual int  getValue();
		virtual void setValue(int value);
};

#endif /* __MODBUSIO_H__ */
