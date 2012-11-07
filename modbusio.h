#ifndef __MODBUSIO_H__
#define __MODBUSIO_H__

class ModbusDevice;

class ModbusIO {
	public:
		ModbusIO(ModbusDevice *dev);
		virtual ~ModbusIO() { }

		virtual int getValue() { return -ENOTSUP; }
		virtual void setValue(int value) { }
	protected:
		ModbusDevice *mDev;
};

class BitInput : public ModbusIO {
		int mBitAddr;
	public:
		BitInput(int bitAddr, ModbusDevice *dev);
		virtual ~BitInput() { }

		virtual int getValue();
};

class BitOutput : public ModbusIO {
		int mBitAddr;
	public:
		BitOutput(int bitAddr, ModbusDevice *dev);
		virtual ~BitOutput() { }

		virtual void setValue(int value);
};

#endif /* __MODBUSIO_H__ */
